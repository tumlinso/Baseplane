#include "lab.hh"
#include <algorithm>
#include <map>
#include <random>
namespace bp_cuda_lab {
static float uniform(std::mt19937& r) {return float(r()%20001)/10000.f-1.f;}
Weights make_weights() {
    Weights w{}; std::mt19937 r(991);
    for(float& x:w.lift)x=uniform(r)*.25f;
    for(float& x:w.w1)x=uniform(r)*.25f;
    for(float& x:w.b1)x=uniform(r)*.1f;
    for(float& x:w.w2)x=uniform(r)*.125f;
    return w; // Deterministic UNTRAINED coefficients, not a learned model.
}
std::vector<Vec> make_values(std::size_t n,std::uint32_t seed) {
    std::mt19937 r(seed); std::vector<Vec> v(n);
    for(auto& t:v)for(float& x:t.x)x=uniform(r);
    return v;
}
std::uint32_t low_mask(unsigned n) {
    if(n>32)throw std::invalid_argument("mask length exceeds warp");
    return n==32?0xffffffffu:(n==0?0u:((1u<<n)-1u));
}
std::vector<Planes> make_planes(std::size_t n,std::uint32_t seed) {
    std::mt19937 r(seed); std::vector<Planes> p(n);
    for(auto& t:p)t={std::uint32_t(r()),std::uint32_t(r()),std::uint32_t(r()),std::uint32_t(r())};
    if(n)p[0].valid=0;
    if(n>1)p[1].valid=~0u;
    return p;
}
Vec lift_scalar(const Planes& p,const Weights& w) {
    unsigned hist[8]{}; unsigned count=0;
    for(unsigned i=0;i<32;++i)if((p.valid>>i)&1u) {
        unsigned k=(((p.a>>i)&1u)<<2)|(((p.b>>i)&1u)<<1)|((p.c>>i)&1u);
        ++hist[k]; ++count;
    }
    Vec y{};
    for(int d=0;d<channels;++d) {
        for(int k=0;k<8;++k)y.x[d]=std::fma(float(hist[k]),w.lift[d*8+k],y.x[d]);
        if(count)y.x[d]/=float(count);
    }
    return y;
}
Vec lift_bitset(const Planes& p,const Weights& w) {
    Vec y{}; unsigned count=unsigned(__builtin_popcount(p.valid));
    for(unsigned k=0;k<8;++k) {
        auto mask=(p.a ^ ((k&4)?0u:~0u)) & (p.b ^ ((k&2)?0u:~0u)) &
                  (p.c ^ ((k&1)?0u:~0u)) & p.valid;
        float n=float(__builtin_popcount(mask));
        for(int d=0;d<channels;++d)y.x[d]=std::fma(n,w.lift[d*8+k],y.x[d]);
    }
    if(count)for(float& x:y.x)x/=float(count);
    return y;
}
std::vector<Affine> make_transforms(const std::vector<Vec>& x,float threshold) {
    std::vector<Affine> t(x.size());
    for(std::size_t i=0;i<x.size();++i) {
        // Current-input test gate, not a genomic annotation or learned boundary claim.
        bool reset=i==0 || std::fabs(x[i].x[0]-x[i-1].x[0])>threshold;
        for(int d=0;d<channels;++d) {
            t[i].a.x[d]=reset?0.f:(.7f+.1f*x[i].x[d]);
            t[i].b.x[d]=.2f*x[i].x[d];
        }
    }
    return t;
}
std::vector<Affine> scan_scalar(const std::vector<Affine>& x) {
    std::vector<Affine> y; y.reserve(x.size()); auto s=identity();
    for(const auto& t:x){s=compose(s,t);y.push_back(s);} return y;
}
std::vector<Affine> scan_tree_reference(const std::vector<Affine>& x) {
    if(x.empty())return {};
    std::vector<Affine> y(x.size()),totals;
    for(std::size_t start=0;start<x.size();start+=32) {
        std::array<Affine,32> a;
        for(int l=0;l<32;++l)a[l]=start+l<x.size()?x[start+l]:identity();
        for(int off=1;off<32;off*=2) {
            auto before=a;
            for(int l=off;l<32;++l)a[l]=compose(before[l-off],before[l]);
        }
        for(int l=0;l<32 && start+l<x.size();++l)y[start+l]=a[l];
        totals.push_back(a[31]);
    }
    if(totals.size()>1) {
        auto carry=scan_tree_reference(totals);
        for(std::size_t i=32;i<y.size();++i)y[i]=compose(carry[i/32-1],y[i]);
    }
    return y;
}
static void mlp(const Vec& x,const Weights& w,float* out) {
    float h[hidden];
    for(int j=0;j<hidden;++j) {
        float z=w.b1[j];for(int k=0;k<channels;++k)z=std::fma(x.x[k],w.w1[j*channels+k],z);
        h[j]=activate(z);
    }
    for(int d=0;d<hidden;++d) {
        float z=0;for(int j=0;j<hidden;++j)z=std::fma(h[j],w.w2[d*hidden+j],z);
        out[d]=cheap(x,d)+activate(z);
    }
}
std::vector<float> refine_scalar(const std::vector<Vec>& x,const Weights& w,float threshold) {
    std::vector<float> y(x.size()*hidden);
    for(std::size_t i=0;i<x.size();++i) {
        if(select(x[i],threshold))mlp(x[i],w,y.data()+i*hidden);
        else for(int d=0;d<hidden;++d)y[i*hidden+d]=cheap(x[i],d);
    }return y;
}
std::vector<float> refine_compacted_reference(const std::vector<Vec>& x,const Weights& w,float threshold) {
    std::vector<float> y(x.size()*hidden);std::vector<std::size_t> queue;
    for(std::size_t i=0;i<x.size();++i) {
        for(int d=0;d<hidden;++d)y[i*hidden+d]=cheap(x[i],d);
        if(select(x[i],threshold))queue.push_back(i);
    }
    // Deliberately reorder the queue: identity restoration, not queue order, defines output.
    std::reverse(queue.begin(),queue.end());
    for(auto i:queue)mlp(x[i],w,y.data()+i*hidden);
    return y;
}
std::uint32_t route_key(const Vec& v) {
    std::uint32_t key=0;for(int d=0;d<channels;++d)key|=std::uint32_t(v.x[d]>0)<<d;return key;
}
std::vector<Match> match_scalar(const std::vector<Vec>& x,const std::vector<std::uint32_t>& keys) {
    if(x.size()!=keys.size())throw std::invalid_argument("key/value size mismatch");
    std::vector<Match> y(x.size());
    for(std::size_t s=0;s<x.size();s+=32)for(std::size_t i=s;i<std::min(s+32,x.size());++i) {
        y[i].leader=0xffffffffu;
        for(std::size_t j=s;j<std::min(s+32,x.size());++j)if(keys[i]==keys[j]) {
            y[i].peers|=1u<<unsigned(j-s);y[i].leader=std::min(y[i].leader,unsigned(j-s));
            for(int d=0;d<channels;++d)y[i].mean.x[d]+=x[j].x[d];
        }
        float n=float(__builtin_popcount(y[i].peers));for(float& v:y[i].mean.x)v/=n;
    }return y;
}
std::vector<Match> match_mask_reference(const std::vector<Vec>& x,const std::vector<std::uint32_t>& keys) {
    if(x.size()!=keys.size())throw std::invalid_argument("key/value size mismatch");
    std::vector<Match> y(x.size());
    for(std::size_t s=0;s<x.size();s+=32) {
        std::map<std::uint32_t,std::vector<unsigned>> groups;
        for(std::size_t i=s;i<std::min(s+32,x.size());++i)groups[keys[i]].push_back(unsigned(i-s));
        for(const auto& group:groups) {
            Match m{};m.leader=group.second.front();
            for(auto j:group.second) {
                m.peers|=1u<<j;for(int d=0;d<channels;++d)m.mean.x[d]+=x[s+j].x[d];
            }
            for(float& v:m.mean.x)v/=float(group.second.size());
            for(auto j:group.second)y[s+j]=m;
        }
    }return y;
}
} // namespace
