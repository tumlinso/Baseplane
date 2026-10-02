#include "reference.hpp"
#include <ce_moon/volta.cuh>
#include <iostream>
using namespace bp_moon;using namespace bp_moon::machine;
void check(cudaError_t e){if(e!=cudaSuccess)throw std::runtime_error(cudaGetErrorString(e));}
template<class T>struct Device{T* p=nullptr;std::size_t n;explicit Device(std::size_t count):n(count){check(cudaMalloc(reinterpret_cast<void**>(&p),n*sizeof(T)));}~Device(){if(p)cudaFree(p);}Device(const Device&)=delete;Device& operator=(const Device&)=delete;void put(const T* src){check(cudaMemcpy(p,src,n*sizeof(T),cudaMemcpyHostToDevice));}std::vector<T> get(){std::vector<T> out(n);check(cudaMemcpy(out.data(),p,n*sizeof(T),cudaMemcpyDeviceToHost));return out;}};
__device__ unsigned plane(u64 word,unsigned bit){unsigned p=0;for(unsigned lane=0;lane<32;++lane)p|=unsigned((word>>(2*lane+bit))&1)<<lane;return p;}
__device__ unsigned count_gc(u64 word,unsigned valid){return __popc((plane(word,0)^plane(word,1))&valid);}
__global__ void gc_fused(const u64* words,const unsigned* valid,unsigned* out,unsigned n){unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i<n)out[i]=count_gc(words[i],valid[i]);}
__global__ void make_planes(const u64* words,uint2* out,unsigned n){unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i<n)out[i]=make_uint2(plane(words[i],0),plane(words[i],1));}
__global__ void gc_planes(const uint2* planes,const unsigned* valid,unsigned* out,unsigned n){unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i<n){auto p=planes[i];out[i]=__popc((p.x^p.y)&valid[i]);}}
__global__ void gc_staged(const u64* words,const unsigned* valid,unsigned* out,unsigned n){
    __shared__ u64 staging[2][128];__shared__ unsigned masks[2][128];unsigned lane=threadIdx.x,base=blockIdx.x*256;
    unsigned i=base+lane;staging[0][lane]=i<n?words[i]:0;masks[0][lane]=i<n?valid[i]:0;__syncthreads();
    for(unsigned tile=0;tile<2;++tile){
        unsigned next=base+(tile+1)*128+lane;
        // Ordinary loads into registers while computing the resident current tile.
        u64 prefetch=tile==0&&next<n?words[next]:0;unsigned nextmask=tile==0&&next<n?valid[next]:0;
        unsigned result=count_gc(staging[tile&1][lane],masks[tile&1][lane]);
        i=base+tile*128+lane;if(i<n)out[i]=result;
        __syncthreads();staging[(tile+1)&1][lane]=prefetch;masks[(tile+1)&1][lane]=nextmask;__syncthreads();
    }
}
__global__ void transpose_shift(const unsigned* in,unsigned* out,unsigned n){unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i<n){unsigned x=in[i],t=(x^(x>>3))&0x0a0a;x^=t^(t<<3);t=(x^(x>>6))&0x00cc;out[i]=(x^t^(t<<6))&0xffff;}}
__constant__ unsigned spread[16]={0,1,16,17,256,257,272,273,4096,4097,4112,4113,4352,4353,4368,4369};
__global__ void transpose_table(const unsigned* in,unsigned* out,unsigned n){unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i<n){unsigned x=in[i],result=0;for(unsigned r=0;r<4;++r)result|=spread[(x>>(4*r))&15]<<r;out[i]=result;}}
// Direct arithmetic comparison at exactly the same sequence-derived coordinates.
__global__ void arithmetic(const float* table,const float2* coordinates,float* out,unsigned n,bool linear){unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i>=n)return;float x=fminf(3.f,fmaxf(0.f,coordinates[i].x-.5f)),y=fminf(3.f,fmaxf(0.f,coordinates[i].y-.5f));int a=int(floorf(x)),b=int(floorf(y));if(!linear){a=int(floorf(x+.5f));b=int(floorf(y+.5f));out[i]=table[b*4+a];return;}int z=min(3,a+1),w=min(3,b+1);float dx=x-a,dy=y-b;out[i]=(1-dy)*((1-dx)*table[b*4+a]+dx*table[b*4+z])+dy*((1-dx)*table[w*4+a]+dx*table[w*4+z]);}
struct Texture{cudaArray_t array=nullptr;cudaTextureObject_t object=0;Texture(const Surface& surface,bool linear){auto channel=cudaCreateChannelDesc<float>();check(cudaMallocArray(&array,&channel,4,4));check(cudaMemcpy2DToArray(array,0,0,surface.table.data(),4*sizeof(float),4*sizeof(float),4,cudaMemcpyHostToDevice));cudaResourceDesc resource{};resource.resType=cudaResourceTypeArray;resource.res.array.array=array;cudaTextureDesc description{};description.addressMode[0]=description.addressMode[1]=cudaAddressModeClamp;description.filterMode=linear?cudaFilterModeLinear:cudaFilterModePoint;description.readMode=cudaReadModeElementType;description.normalizedCoords=0;check(cudaCreateTextureObject(&object,&resource,&description,nullptr));}~Texture(){if(object)cudaDestroyTextureObject(object);if(array)cudaFreeArray(array);}Texture(const Texture&)=delete;};
int main(int argc,char** argv){if(argc!=3||std::string(argv[1])!="--run"){std::cerr<<"Explicit assigned device required: --run DEVICE\n";return 2;}try{
    check(cudaSetDevice(std::stoi(argv[2])));std::string text;for(unsigned i=0;i<9001;++i)text+="ACGTNGCA"[i%8];Input input{PackedFixture(text),{31,7,500,4,Strand::forward,text.size()}};
    const unsigned n=unsigned(input.sequence.words.size());Device<u64> words(n);words.put(input.sequence.words.data());Device<unsigned> valid(n);valid.put(input.sequence.valid.data());Device<unsigned> out(n);Device<uint2> planes(n);
    auto expected=fused(input);gc_fused<<<(n+127)/128,128>>>(words.p,valid.p,out.p,n);check(cudaGetLastError());if(out.get()!=expected)throw std::runtime_error("fused GC");
    gc_staged<<<(n+255)/256,128>>>(words.p,valid.p,out.p,n);check(cudaGetLastError());if(out.get()!=expected)throw std::runtime_error("double buffered GC");
    make_planes<<<(n+127)/128,128>>>(words.p,planes.p,n);check(cudaGetLastError());gc_planes<<<(n+127)/128,128>>>(planes.p,valid.p,out.p,n);check(cudaGetLastError());if(out.get()!=expected)throw std::runtime_error("resident planes GC");
    std::vector<unsigned> perm_input;for(auto word:input.sequence.words)perm_input.push_back(unsigned(word&0xffff));Device<unsigned> perm(n);perm.put(perm_input.data());
    for(unsigned mode=0;mode<2;++mode){if(mode==0)transpose_shift<<<(n+127)/128,128>>>(perm.p,out.p,n);else transpose_table<<<(n+127)/128,128>>>(perm.p,out.p,n);check(cudaGetLastError());auto actual=out.get();for(unsigned i=0;i<n;++i)if(actual[i]!=transpose_reference(perm_input[i]))throw std::runtime_error("transpose survivor");}
    std::array<std::int8_t,4> weights{2,-1,3,-2};std::vector<int> a(n),b(n,packed(weights));for(unsigned i=0;i<n;++i)a[i]=packed(features(input,i).bytes);Device<int> da(n),db(n),dot(n);da.put(a.data());db.put(b.data());ce_moon_cuda::packed_dot<<<(n+127)/128,128>>>(da.p,db.p,dot.p,n);check(cudaGetLastError());auto score=dot.get();for(unsigned i=0;i<n;++i)if(score[i]!=scalar_dot(features(input,i),weights))throw std::runtime_error("DP4A routing");
    std::vector<float> lanes(32);for(unsigned i=0;i<32;++i)lanes[i]=input.sequence.is_valid(i)?float((input.sequence.words[0]>>(2*i))&3):0.f;Device<float> lane_in(32),lane_mix(32),lane_back(32);lane_in.put(lanes.data());ce_moon_cuda::butterfly32<<<1,32>>>(lane_in.p,lane_mix.p,1);check(cudaGetLastError());ce_moon_cuda::butterfly32<<<1,32>>>(lane_mix.p,lane_back.p,1);check(cudaGetLastError());auto roundtrip=lane_back.get();for(unsigned i=0;i<32;++i)if(roundtrip[i]/32!=lanes[i])throw std::runtime_error("warp butterfly roundtrip");
    Surface bilinear{4,4,{}},jump{4,4,{}};for(unsigned y=0;y<4;++y)for(unsigned x=0;x<4;++x){bilinear.table.push_back(float(x+2*y+x*y));jump.table.push_back(x<2?0.f:1.f);}
    auto sequence_xy=coordinates(input,0);std::vector<float2> xy{{.75f,2.f},{2.f,1.5f},{-.5f,-.5f},{4.5f,4.5f},{sequence_xy.first+.5f,sequence_xy.second+.5f}};
    Device<float2> coords(xy.size());coords.put(xy.data());Device<float> response_out(xy.size()),direct(xy.size()),table(16);float max_error=0;
    for(const auto* surface:{&bilinear,&jump})for(bool linear:{false,true}){Texture texture(*surface,linear);table.put(surface->table.data());ce_moon_cuda::texture_response<<<1,32>>>(texture.object,coords.p,response_out.p,unsigned(xy.size()));check(cudaGetLastError());arithmetic<<<1,32>>>(table.p,coords.p,direct.p,unsigned(xy.size()),linear);check(cudaGetLastError());auto actual=response_out.get(),plain=direct.get();for(unsigned i=0;i<xy.size();++i){float oracle=response(*surface,xy[i].x-.5f,xy[i].y-.5f,linear);float error=std::abs(actual[i]-oracle);max_error=std::max(max_error,error);if(std::abs(plain[i]-oracle)>1e-5f||error>(linear?.04f:1e-5f))throw std::runtime_error("texture arithmetic/precision");}}
    std::cout<<"E45 max_texture_error="<<max_error<<" E46 DP4A words="<<n<<" butterfly roundtrip passed E47 two transposes passed E48 three GC paths passed\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
