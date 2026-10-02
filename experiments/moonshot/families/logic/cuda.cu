#include <cuda_runtime.h>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
namespace {
template<unsigned LUT> __device__ unsigned lut(unsigned a,unsigned b,unsigned c){
 unsigned out;asm("lop3.b32 %0, %1, %2, %3, %4;":"=r"(out):"r"(a),"r"(b),"r"(c),"n"(LUT));return out;
}
// One thread owns a word. All buffers are caller-owned and nonaliasing.
__global__ void circuit(const unsigned* a,const unsigned* b,const unsigned* c,const unsigned* valid,unsigned* out,unsigned words){
 unsigned w=blockIdx.x*blockDim.x+threadIdx.x;if(w<words)out[w]=lut<0x80>(lut<0x96>(a[w],b[w],c[w]),b[w],c[w])&valid[w];
}
__global__ void evidence(const unsigned* supports,const unsigned* valid,unsigned* planes,unsigned* overflow,unsigned words,unsigned votes){
 unsigned w=blockIdx.x*blockDim.x+threadIdx.x;if(w>=words)return;
 unsigned p[4]={},ov=0;for(unsigned v=0;v<votes;++v){unsigned carry=supports[v*words+w]&valid[w];
 for(unsigned b=0;b<4;++b){unsigned next=p[b]&carry;p[b]^=carry;carry=next;}ov|=carry;}
 for(unsigned b=0;b<4;++b)planes[b*words+w]=p[b];overflow[w]=ov;
}
__device__ unsigned pull(unsigned a,unsigned next,unsigned shift){return shift?(a>>shift)|(next<<(32-shift)):a;}
// Exact A-C-G grammar uses validity-masked base masks; halo word never owns anchor.
__global__ void grammar(const unsigned* a,const unsigned* c,const unsigned* g,unsigned* out,unsigned words){
 unsigned w=blockIdx.x*blockDim.x+threadIdx.x;if(w>=words)return;
 out[w]=a[w]&pull(c[w],w+1<words?c[w+1]:0,1)&pull(g[w],w+1<words?g[w+1]:0,2);
}
__global__ void query(const unsigned* parent,const unsigned* question,unsigned* coordinates,unsigned* ranks,unsigned words){
 unsigned w=blockIdx.x*blockDim.x+threadIdx.x;if(w>=words)return;
 unsigned selected=parent[w]&question[w],r=0;
 while(selected){unsigned lane=__ffs(selected)-1;coordinates[w*32+r]=w*32+lane;
 ranks[w*32+r]=__popc(parent[w]&((1u<<lane)-1u));++r;selected&=selected-1;}
}
void ok(cudaError_t e){if(e!=cudaSuccess)throw std::runtime_error(cudaGetErrorString(e));}
void require(bool b){if(!b)throw std::runtime_error("CUDA semantic mismatch");}
}
int main(int argc,char** argv){
 if(argc!=3||std::strcmp(argv[1],"--run")){std::puts("Use --run DEVICE after an assigned GPU lease; no automatic launch.");return 0;}
 try{
  ok(cudaSetDevice(std::stoi(argv[2])));cudaStream_t stream;ok(cudaStreamCreate(&stream));
  unsigned host[160]={};unsigned* device;ok(cudaMalloc(&device,sizeof(host)));
  host[0]=0x80000001;host[2]=~0u;host[4]=~0u;host[6]=0x7fffffff;
  ok(cudaMemcpyAsync(device,host,sizeof(host),cudaMemcpyHostToDevice,stream));
  circuit<<<1,32,0,stream>>>(device,device+2,device+4,device+6,device+8,1);ok(cudaGetLastError());
  ok(cudaMemcpyAsync(host,device,sizeof(host),cudaMemcpyDeviceToHost,stream));ok(cudaStreamSynchronize(stream));require(host[8]==1);
  for(unsigned v=0;v<16;++v)host[v]=~0u;host[16]=0x7fffffff;
  ok(cudaMemcpyAsync(device,host,sizeof(host),cudaMemcpyHostToDevice,stream));
  evidence<<<1,32,0,stream>>>(device,device+16,device+20,device+24,1,16);ok(cudaGetLastError());
  ok(cudaMemcpyAsync(host,device,sizeof(host),cudaMemcpyDeviceToHost,stream));ok(cudaStreamSynchronize(stream));
  require(host[20]==0&&host[21]==0&&host[22]==0&&host[23]==0&&host[24]==0x7fffffff);
  std::memset(host,0,sizeof(host));host[0]=0x80000000;host[3]=1;host[5]=2;
  ok(cudaMemcpyAsync(device,host,sizeof(host),cudaMemcpyHostToDevice,stream));
  grammar<<<1,32,0,stream>>>(device,device+2,device+4,device+6,2);ok(cudaGetLastError());
  ok(cudaMemcpyAsync(host,device,sizeof(host),cudaMemcpyDeviceToHost,stream));ok(cudaStreamSynchronize(stream));require(host[6]==0x80000000&&host[7]==0);
  host[0]=0x80000022;host[1]=0x80000002;
  ok(cudaMemcpyAsync(device,host,sizeof(host),cudaMemcpyHostToDevice,stream));
  query<<<1,32,0,stream>>>(device,device+1,device+2,device+34,1);ok(cudaGetLastError());
  ok(cudaMemcpyAsync(host,device,sizeof(host),cudaMemcpyDeviceToHost,stream));ok(cudaStreamSynchronize(stream));
  require(host[2]==1&&host[3]==31&&host[34]==0&&host[35]==2);
  ok(cudaFree(device));ok(cudaStreamDestroy(stream));std::puts("{\"status\":\"pass\",\"cuda_mechanisms\":4}");return 0;
 }catch(const std::exception& e){std::fprintf(stderr,"%s\n",e.what());return 1;}
}
