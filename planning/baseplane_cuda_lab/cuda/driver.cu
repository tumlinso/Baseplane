#include "kernels.cuh"
#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
using namespace bp_cuda_lab;
static void ck(cudaError_t e){if(e!=cudaSuccess)throw std::runtime_error(cudaGetErrorString(e));}
template<class T> struct Buffer {
    T* p=nullptr;std::size_t n;
    explicit Buffer(std::size_t size):n(size){ck(cudaMalloc(reinterpret_cast<void**>(&p),std::max<std::size_t>(1,n)*sizeof(T)));}
    explicit Buffer(const std::vector<T>& x):Buffer(x.size()){if(n)ck(cudaMemcpy(p,x.data(),n*sizeof(T),cudaMemcpyHostToDevice));}
    ~Buffer(){if(p)cudaFree(p);}Buffer(const Buffer&)=delete;Buffer& operator=(const Buffer&)=delete;
    std::vector<T> read()const{std::vector<T> x(n);if(n)ck(cudaMemcpy(x.data(),p,n*sizeof(T),cudaMemcpyDeviceToHost));return x;}
};
static void near(float a,float b){if(!std::isfinite(a)||!std::isfinite(b)||fabsf(a-b)>4e-5f*(1+fabsf(a)+fabsf(b)))throw std::runtime_error("device/scalar mismatch");}
static void eq(Vec a,Vec b){for(int d=0;d<channels;++d)near(a.x[d],b.x[d]);}
static void eq(Affine a,Affine b){eq(a.a,b.a);eq(a.b,b.b);}
static unsigned blocks(unsigned n){return (n+127)/128;}
static unsigned bounded_number(const char* text,unsigned maximum) {
    const std::string value(text);std::size_t used=0;
    if(value.empty() || value[0]=='-')throw std::invalid_argument("expected a nonnegative integer");
    const auto parsed=std::stoull(value,&used);
    if(used!=value.size() || parsed>maximum)throw std::invalid_argument("bounded prototype size exceeded");
    return static_cast<unsigned>(parsed);
}
static void finish(){ck(cudaGetLastError());ck(cudaDeviceSynchronize());}
static void measured(const char* experiment,const char* variant,unsigned n,unsigned iters,const std::function<void()>& fn,const std::string& context=""){
    if(!iters)return;for(int i=0;i<3;++i)fn();finish();
    cudaEvent_t a,b;ck(cudaEventCreate(&a));ck(cudaEventCreate(&b));std::vector<float> samples;
    for(unsigned i=0;i<iters;++i){ck(cudaEventRecord(a));fn();ck(cudaEventRecord(b));ck(cudaEventSynchronize(b));float ms;ck(cudaEventElapsedTime(&ms,a,b));samples.push_back(ms);}
    ck(cudaEventDestroy(a));ck(cudaEventDestroy(b));std::sort(samples.begin(),samples.end());
    std::cout<<"{\"phase\":\"device_resident_pipeline\",\"case\":\""<<experiment<<"\",\"variant\":\""<<variant<<"\",\"n\":"<<n<<",\"iterations\":"<<iters<<",\"median_ms\":"<<samples[samples.size()/2]<<",\"p95_ms\":"<<samples[std::min(samples.size()-1,std::size_t(samples.size()*.95))]<<context<<",\"scalar_checked\":true}\n";
}
struct ScanTree {
    unsigned n,ntiles;Buffer<Affine> prefix,totals;std::unique_ptr<ScanTree> upper;
    explicit ScanTree(unsigned len):n(len),ntiles((len+31)/32),prefix(len),totals(ntiles){if(ntiles>1)upper=std::make_unique<ScanTree>(ntiles);}
    void run(const Affine* in){if(!n)return;scan_tiles<<<blocks(n),128>>>(in,prefix.p,totals.p,n);if(upper){upper->run(totals.p);scan_carry<<<blocks(n),128>>>(prefix.p,upper->prefix.p,n);}}
};
static void test_bitlift(unsigned n,unsigned iters){
    auto h=make_planes(n);auto w=make_weights();Buffer<Planes> in(h);Buffer<Vec> out(n);Buffer<Weights> dw(std::vector<Weights>{w});
    std::vector<Vec> expect;for(auto x:h)expect.push_back(lift_scalar(x,w));
    auto a=[&]{if(n)bitlift_warp<<<blocks(n*32),128>>>(in.p,out.p,dw.p,n);};
    auto b=[&]{if(n)bitlift_thread<<<blocks(n),128>>>(in.p,out.p,dw.p,n);};
    for(auto fn:{std::function<void()>(a),std::function<void()>(b)}){fn();finish();auto got=out.read();for(unsigned i=0;i<n;++i)eq(expect[i],got[i]);}
    measured("bitlift","warp_lop3_popc_fp",n,iters,a);measured("bitlift","thread_bitset_fp",n,iters,b);
}
static void test_scan(unsigned n,unsigned iters){
    auto input=make_transforms(make_values(n),.6f);auto expect=scan_scalar(input);Buffer<Affine> in(input);ScanTree tree(n);
    auto run=[&]{tree.run(in.p);};run();finish();auto got=tree.prefix.read();for(unsigned i=0;i<n;++i)eq(expect[i],got[i]);
    measured("scan","multilevel_affine_shuffle_scan",n,iters,run);
    // A CUB scan competitor is intentionally an epic deliverable, not hidden here.
}
static void test_rethread(unsigned n,unsigned iters){
    auto x=make_values(n);auto w=make_weights();Buffer<Vec> in(x);Buffer<Weights> dw(std::vector<Weights>{w});
    Buffer<float> out(std::size_t(n)*hidden);Buffer<unsigned> queue(n),count(1),dropped(1);
    int dev;cudaDeviceProp prop;ck(cudaGetDevice(&dev));ck(cudaGetDeviceProperties(&prop,dev));
    unsigned resident=std::max(1,prop.multiProcessorCount*2);
    for(float threshold:{-2.f,0.f,2.f}){
        auto expect=refine_scalar(x,w,threshold);
        auto compact=[&]{ck(cudaMemsetAsync(count.p,0,sizeof(unsigned)));ck(cudaMemsetAsync(dropped.p,0,sizeof(unsigned)));if(n){cheap_all<<<blocks(n),128>>>(in.p,out.p,n);select_queue<<<blocks(n),128>>>(in.p,n,threshold,queue.p,n,count.p,dropped.p);refine_queue<<<resident,128>>>(in.p,queue.p,count.p,n,dw.p,out.p);}};
        auto warp=[&]{if(n){cheap_all<<<blocks(n),128>>>(in.p,out.p,n);refine_warp_direct<<<resident,128>>>(in.p,n,threshold,dw.p,out.p);}};
        auto thread=[&]{if(n)refine_thread_direct<<<blocks(n),128>>>(in.p,n,threshold,dw.p,out.p);};
        for(auto fn:{std::function<void()>(compact),std::function<void()>(warp),std::function<void()>(thread)}){fn();finish();auto y=out.read();for(std::size_t i=0;i<y.size();++i)near(expect[i],y[i]);}
        compact();finish();unsigned expected=0;for(auto v:x)expected+=select(v,threshold);if(count.read()[0]!=expected||dropped.read()[0]!=0)throw std::runtime_error("queue count mismatch");
        // Each timing record includes its own density context.
        std::cout<<"{\"case\":\"rethread\",\"n\":"<<n<<",\"threshold\":"<<threshold<<",\"selected\":"<<expected<<"}\n";
        const auto context=",\"threshold\":"+std::to_string(threshold)+",\"selected\":"+std::to_string(expected);
        measured("rethread","compacted_warp_mlp",n,iters,compact,context);measured("rethread","uncompacted_warp_mlp",n,iters,warp,context);measured("rethread","thread_mlp",n,iters,thread,context);
    }
    // Explicit overflow test. Never consume a truncated queue as a successful model result.
    if(n){unsigned cap=n/2;ck(cudaMemset(count.p,0,4));ck(cudaMemset(dropped.p,0,4));select_queue<<<blocks(n),128>>>(in.p,n,-2.f,queue.p,cap,count.p,dropped.p);finish();if(count.read()[0]!=n||dropped.read()[0]!=n-cap)throw std::runtime_error("overflow not accounted");}
}
static void test_rendezvous(unsigned n,unsigned iters){
    auto x=make_values(n);Buffer<Vec> in(x);Buffer<Match> out(n);
    for(int mode=0;mode<3;++mode){
        std::vector<unsigned> key(n);for(unsigned i=0;i<n;++i)key[i]=mode==0?route_key(x[i]):mode==1?0xffffffffu:i;
        auto expect=match_scalar(x,key);Buffer<unsigned> dk(key);
        auto a=[&]{if(n)rendezvous<<<blocks(n),128>>>(in.p,dk.p,out.p,n);};
        auto b=[&]{if(n)rendezvous_pairwise<<<blocks(n),128>>>(in.p,dk.p,out.p,n);};
        for(auto fn:{std::function<void()>(a),std::function<void()>(b)}){fn();finish();auto got=out.read();for(unsigned i=0;i<n;++i){if(got[i].peers!=expect[i].peers||got[i].leader!=expect[i].leader)throw std::runtime_error("peer identity mismatch");eq(got[i].mean,expect[i].mean);}}
        std::cout<<"{\"case\":\"rendezvous\",\"n\":"<<n<<",\"key_mode\":"<<mode<<"}\n";
        const auto context=",\"key_mode\":"+std::to_string(mode);
        measured("rendezvous","match_any_peer_shuffle",n,iters,a,context);measured("rendezvous","pairwise_shuffle",n,iters,b,context);
    }
}
int main(int argc,char** argv){
    try{
        std::string name=argc>1?argv[1]:"all";unsigned n=argc>2?bounded_number(argv[2],1u<<20):1025u;unsigned iters=argc>3?bounded_number(argv[3],10000):0u;
        if(n>(1u<<20)||iters>10000)throw std::invalid_argument("bounded prototype limits: n<=1048576, iterations<=10000");
        int count=0;auto status=cudaGetDeviceCount(&count);if(status!=cudaSuccess||!count){std::cerr<<"CUDA device unavailable\n";return 77;}
        int dev;cudaDeviceProp prop;ck(cudaGetDevice(&dev));ck(cudaGetDeviceProperties(&prop,dev));if(prop.major<7)throw std::runtime_error("requires sm_70+");
        std::cout<<"{\"device\":\""<<prop.name<<"\",\"sm\":"<<prop.major*10+prop.minor<<",\"cudart\":"<<CUDART_VERSION<<",\"untrained_fixture_weights\":true}\n";
        bool known=false;
        if(name=="all"||name=="bitlift"){test_bitlift(n,iters);known=true;}
        if(name=="all"||name=="scan"){test_scan(n,iters);known=true;}
        if(name=="all"||name=="rethread"){test_rethread(n,iters);known=true;}
        if(name=="all"||name=="rendezvous"){test_rendezvous(n,iters);known=true;}
        if(!known)throw std::invalid_argument("unknown case");
        std::cout<<"{\"phase\":\"cuda_correctness\",\"case\":\""<<name<<"\",\"n\":"<<n<<",\"passed\":true}\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
