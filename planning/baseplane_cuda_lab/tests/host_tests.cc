#include "lab.hh"
#include <iostream>
#include <random>
#include <string>
#include <limits>
#include <algorithm>
using namespace bp_cuda_lab;
static unsigned long long checks=0;
static void require(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}
static void near(float a,float b){require(std::isfinite(a)&&std::isfinite(b)&&std::fabs(a-b)<3e-5f*(1+std::fabs(a)+std::fabs(b)),"FP mismatch");}
static void equal(Vec a,Vec b){for(int d=0;d<channels;++d)near(a.x[d],b.x[d]);}
static void equal(Affine a,Affine b){equal(a.a,b.a);equal(a.b,b.b);}
static void bitlift_tests(){
    auto w=make_weights();
    for(unsigned count=0;count<=32;++count)for(unsigned k=0;k<8;++k){
        Planes p{(k&4)?~0u:0u,(k&2)?~0u:0u,(k&1)?~0u:0u,low_mask(count)};
        equal(lift_scalar(p,w),lift_bitset(p,w));
        Vec expect{};if(count)for(int d=0;d<channels;++d)expect.x[d]=w.lift[d*8+k];equal(expect,lift_scalar(p,w));
    }
    for(auto p:make_planes(2000))equal(lift_scalar(p,w),lift_bitset(p,w));
    // A deliberate loss witness: permutation of exact predicate positions is
    // invisible to a histogram. These vectors are NOT lossless sequence storage.
    Planes a{0x0000ffffu,0xf0f0f0f0u,0xaaaaaaaau,~0u};
    auto rotate=[](unsigned x){return (x<<1)|(x>>31);};
    Planes b{rotate(a.a),rotate(a.b),rotate(a.c),~0u};
    require(a.a!=b.a,"loss witness must change input");equal(lift_scalar(a,w),lift_scalar(b,w));
    bool rejected=false;try{low_mask(33);}catch(const std::invalid_argument&){rejected=true;}require(rejected,"mask overflow unchecked");
}
static void scan_tests(){
    for(unsigned n:{0,1,17,31,32,33,63,64,65,1023,1024,1025,4097,32769}){
        auto v=make_values(n);
        for(float th:{0.f,.6f,3.f}){
            auto x=make_transforms(v,th),a=scan_scalar(x),b=scan_tree_reference(x);
            require(a.size()==b.size(),"scan size");for(unsigned i=0;i<n;++i)equal(a[i],b[i]);
        }
    }
    auto x=make_transforms(make_values(6),3.f);
    // Restore nonzero A to test operator summaries as well as the initial-state trajectory.
    for(auto& t:x)for(int d=0;d<channels;++d)t.a.x[d]=.8f;
    equal(compose(compose(x[0],x[1]),x[2]),compose(x[0],compose(x[1],x[2])));
    equal(compose(identity(),x[0]),x[0]);equal(compose(x[0],identity()),x[0]);
    auto ab=compose(x[0],x[1]),ba=compose(x[1],x[0]);bool differs=false;
    for(int d=0;d<channels;++d)differs|=std::fabs(ab.b.x[d]-ba.b.x[d])>1e-5f;
    require(differs,"composition incorrectly commutative");
    // Runtime reset breaks dependence on a very different incoming state.
    auto reset=x[3];for(float& a:reset.a.x)a=0;
    equal(compose(x[0],reset),compose(x[1],reset));
}
static void rethread_tests(){
    auto w=make_weights();
    for(unsigned n:{0,1,17,31,32,33,65,257})for(float th:{-2.f,0.f,2.f}){
        auto x=make_values(n);auto a=refine_scalar(x,w,th),b=refine_compacted_reference(x,w,th);
        require(a.size()==std::size_t(n)*hidden,"refinement lost records");
        for(std::size_t i=0;i<a.size();++i)near(a[i],b[i]);
        if(th==2.f)for(unsigned i=0;i<n;++i)for(int d=0;d<hidden;++d)near(a[i*hidden+d],cheap(x[i],d));
    }
}
static void rendezvous_tests(){
    for(unsigned n:{0,1,17,31,32,33,63,64,65,257}){
        auto x=make_values(n);std::vector<unsigned> keys(n);
        for(int mode=0;mode<3;++mode){
            for(unsigned i=0;i<n;++i)keys[i]=mode==0?route_key(x[i]):mode==1?0xffffffffu:i;
            auto a=match_scalar(x,keys),b=match_mask_reference(x,keys);
            for(unsigned i=0;i<n;++i){require(a[i].peers==b[i].peers,"peer mismatch");require(a[i].leader==b[i].leader,"leader mismatch");equal(a[i].mean,b[i].mean);require((a[i].peers>>(i%32))&1u,"self missing");if(mode==2)equal(a[i].mean,x[i]);}
        }
    }
    bool rejected=false;try{match_scalar(make_values(2),{1});}catch(const std::invalid_argument&){rejected=true;}require(rejected,"invalid peer contract");
}
int main(int argc,char** argv){
    std::string which=argc>1?argv[1]:"all";
    try{
        bool known=false;
        if(which=="all"||which=="bitlift"){bitlift_tests();known=true;}
        if(which=="all"||which=="scan"){scan_tests();known=true;}
        if(which=="all"||which=="rethread"){rethread_tests();known=true;}
        if(which=="all"||which=="rendezvous"){rendezvous_tests();known=true;}
        if(!known)throw std::invalid_argument("unknown experiment");
        std::cout<<"{\"phase\":\"host_reference\",\"case\":\""<<which<<"\",\"assertions\":"<<checks<<",\"passed\":true}\n";return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
