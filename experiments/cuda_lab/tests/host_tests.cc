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
    auto order_label=[](const Planes& p){return ((p.a&1u)<<2)|((p.b&1u)<<1)|(p.c&1u);};
    require(order_label(a)!=order_label(b),"loss witness must change order-sensitive label");
    for(auto p:make_planes(257)){auto packed=pack_window(p);auto restored=planes_from_packed(packed);require(restored.valid==p.valid,"packed validity changed");require(restored.a==p.a&&restored.b==p.b&&restored.c==(p.a&p.b),"packed adapter mismatch");}
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
    auto boundary=make_values(70);for(auto& v:boundary)v.x[0]=0.f;
    boundary[31].x[0]=1.f;boundary[32].x[0]=0.f;boundary[33].x[0]=1.f;boundary[34].x[0]=1.f;
    auto gated=make_transforms(boundary,.5f);
    for(unsigned i:{31u,32u,33u})for(float a:gated[i].a.x)near(a,0.f);
    for(float a:gated[34].a.x)require(a!=0.f,"hardware tile caused a semantic reset");
    auto soft=make_values(2);soft[0].x[0]=soft[1].x[0]=0.f;auto retained=make_transforms(soft,1.f);
    for(float a:retained[1].a.x)require(a>0.f&&a<1.f,"soft retention missing");
}
static void rethread_tests(){
    auto w=make_weights();
    for(unsigned n:{0,1,17,31,32,33,65,257})for(float th:{-2.f,0.f,2.f}){
        auto x=make_values(n);auto a=refine_scalar(x,w,th),b=refine_compacted_reference(x,w,th);
        require(a.size()==std::size_t(n)*hidden,"refinement lost records");
        for(std::size_t i=0;i<a.size();++i)near(a[i],b[i]);
        if(th==2.f)for(unsigned i=0;i<n;++i)for(int d=0;d<hidden;++d)near(a[i*hidden+d],cheap(x[i],d));
    }
    // A coarse floating record retains an explicit source id across selection
    // and refinement; queue position is deliberately unrelated to output slot.
    auto fine=make_values(9);std::vector<Vec> coarse(3);std::vector<unsigned> source(3);
    for(unsigned g=0;g<3;++g){source[g]=2-g;for(int d=0;d<channels;++d){float sum=0;unsigned end=std::min(4*g+4,9u);for(unsigned i=4*g;i<end;++i)sum+=cheap(fine[i],d);coarse[g].x[d]=sum/float(end-4*g);}}
    auto refined=refine_compacted_reference(coarse,w,0.f);std::vector<float> routed(refined.size());
    for(unsigned i=0;i<3;++i)std::copy_n(refined.data()+std::size_t(i)*hidden,hidden,routed.data()+std::size_t(source[i])*hidden);
    for(unsigned i=0;i<3;++i)for(int d=0;d<hidden;++d)near(routed[std::size_t(source[i])*hidden+d],refined[std::size_t(i)*hidden+d]);
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
    // Controlled placement: the same distant pair is invisible across lane 31/32,
    // but becomes a local peer pair when moved into one warp packet.
    auto v=make_values(64);std::vector<unsigned> k(64);for(unsigned i=0;i<64;++i)k[i]=i;k[31]=k[32]=0xdeadbeefu;
    auto split=match_scalar(v,k);require(__builtin_popcount(split[31].peers)==1&&__builtin_popcount(split[32].peers)==1,"warp boundary should split candidate pair");
    std::vector<Vec> moved_v(64);std::vector<unsigned> moved_k(64),source(64);for(unsigned i=0;i<64;++i){unsigned from=i==30?32:(i==32?30:i);moved_v[i]=v[from];moved_k[i]=k[from];source[i]=from;}
    auto moved=match_scalar(moved_v,moved_k);require(__builtin_popcount(moved[30].peers)==2&&__builtin_popcount(moved[31].peers)==2,"within-warp placement should restore pair");
    require(source[30]==32&&source[31]==31,"placement source map lost");
    auto global=match_supertile_reference(v,k);require(global[31].count==2&&global[32].count==2,"all-pairs candidate set missed cross-warp partners");
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
