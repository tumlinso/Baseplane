#pragma once
#include "lab.hh"
#include <cuda_runtime.h>
#include <cub/device/device_select.cuh>
#include <cub/device/device_scan.cuh>
#include <cub/device/device_radix_sort.cuh>
#include <thrust/iterator/counting_iterator.h>
namespace bp_cuda_lab {
// Every launch uses blockDim.x multiple of 32. Kernels never allocate, choose
// devices, synchronize the host or manufacture a stream. Caller owns storage.
constexpr unsigned full=0xffffffffu;
__device__ inline unsigned and3_ptx(unsigned a,unsigned b,unsigned c) {
    unsigned d; asm("lop3.b32 %0, %1, %2, %3, 0x80;" : "=r"(d) : "r"(a),"r"(b),"r"(c));return d;
}
__global__ void packed_to_planes(const PackedWindow* packed,Planes* out,unsigned n) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i>=n)return;PackedWindow p=packed[i];Planes v{};v.valid=p.valid;
    for(unsigned b=0;b<32;++b){unsigned code=unsigned((p.bases>>(2*b))&3u);if(code&2u)v.a|=1u<<b;if(code&1u)v.b|=1u<<b;if(code==3u)v.c|=1u<<b;}
    out[i]=v;
}
__global__ void threshold_records_to_planes(const Vec* lower,unsigned lower_n,const unsigned* child_begin,const unsigned* child_end,
                                             Planes* out,unsigned* source_begin,unsigned* source_end,
                                             unsigned windows,float t0,float t1,float t2) {
    unsigned tile=blockIdx.x*blockDim.x+threadIdx.x;if(tile>=windows)return;
    unsigned first=tile*32,end=min(first+32,lower_n);Planes p{};
    for(unsigned lane=0;lane<end-first;++lane){const Vec v=lower[first+lane];unsigned bit=1u<<lane;
        if(v.x[0]>t0)p.a|=bit;if(v.x[1]>t1)p.b|=bit;if(v.x[2]>t2)p.c|=bit;p.valid|=bit;
    }
    out[tile]=p;source_begin[tile]=child_begin[first];source_end[tile]=child_end[end-1];
}
// E1: eight exact 3-bit minterms x four FP output channels fill one warp.
__global__ void bitlift_warp(const Planes* in,Vec* out,const Weights* w,unsigned n) {
    unsigned tid=blockIdx.x*blockDim.x+threadIdx.x, tile=tid>>5, lane=tid&31;
    if(tile>=n)return; // warp-uniform
    Planes p=in[tile]; unsigned pattern=lane&7, d=lane>>3;
    unsigned a=p.a ^ (0u-unsigned(!(pattern&4)));
    unsigned b=p.b ^ (0u-unsigned(!(pattern&2)));
    unsigned c=p.c ^ (0u-unsigned(!(pattern&1)));
    float z=float(__popc(and3_ptx(a,b,c)&p.valid))*w->lift[d*8+pattern];
    // width=8 partitions channels but ALL 32 lanes participate in each sync call.
    for(int off=4;off;off>>=1)z+=__shfl_down_sync(full,z,off,8);
    if(pattern==0)out[tile].x[d]=__popc(p.valid)?z/float(__popc(p.valid)):0.f;
}
__global__ void bitlift_thread(const Planes* in,Vec* out,const Weights* w,unsigned n) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i>=n)return;
    Planes p=in[i];Vec y{};
    for(unsigned k=0;k<8;++k) {
        unsigned m=(p.a ^ (0u-unsigned(!(k&4))))&(p.b ^ (0u-unsigned(!(k&2))))&
                   (p.c ^ (0u-unsigned(!(k&1))))&p.valid;
        for(int d=0;d<channels;++d)y.x[d]=fmaf(float(__popc(m)),w->lift[d*8+k],y.x[d]);
    }
    if(__popc(p.valid))for(int d=0;d<channels;++d)y.x[d]/=float(__popc(p.valid));out[i]=y;
}
// E2: the reset is data (a=0), not a divergent program. Tile totals carry the
// exact same algebra to the next scale; hardware tile boundaries are NOT resets.
__global__ void produce_affine(const Vec* x,Affine* out,unsigned n,float gate) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i>=n)return;
    bool reset=i==0 || fabsf(x[i].x[0]-x[i-1].x[0])>gate;
    for(int d=0;d<channels;++d){out[i].a.x[d]=reset?0.f:(.7f+.1f*x[i].x[d]);out[i].b.x[d]=.2f*x[i].x[d];}
}
struct AffineCompose {
    __host__ __device__ Affine operator()(const Affine& left,const Affine& right) const {return compose(left,right);}
};
__global__ void summarize_affine_groups(const Affine* fine,Affine* coarse,unsigned* source_begin,
                                        unsigned* source_end,unsigned n,unsigned groups) {
    unsigned g=blockIdx.x*blockDim.x+threadIdx.x;if(g>=groups)return;
    unsigned first=4*g,end=min(first+4,n);Affine acc=identity();
    for(unsigned i=first;i<end;++i)acc=compose(acc,fine[i]);
    coarse[g]=acc;source_begin[g]=first;source_end[g]=end;
}
__global__ void scan_tiles(const Affine* in,Affine* out,Affine* totals,unsigned n) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x,lane=i&31,tile=i>>5;
    if(tile>=(n+31u)/32u)return; // uniform within each warp
    Affine t=i<n?in[i]:identity();
    for(int off=1;off<32;off<<=1) {
        Affine left;
        for(int d=0;d<channels;++d){left.a.x[d]=__shfl_up_sync(full,t.a.x[d],off);left.b.x[d]=__shfl_up_sync(full,t.b.x[d],off);}
        if(lane>=unsigned(off))t=compose(left,t);
    }
    if(i<n)out[i]=t;if(lane==31)totals[tile]=t;
}
__global__ void scan_carry(Affine* out,const Affine* upper,unsigned n) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i>=n||i<32)return;
    out[i]=compose(upper[i/32-1],out[i]);
}
// E3: level-synchronous producer. Counter is attempted count; overflow explicit.
__global__ void select_queue(const Vec* x,unsigned n,float threshold,unsigned* queue,
                             unsigned capacity,unsigned* count,unsigned* dropped) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x,lane=i&31;
    bool yes=i<n && select(x[i],threshold);
    unsigned mask=__ballot_sync(full,yes), base=0, k=__popc(mask);
    if(lane==0 && k)base=atomicAdd(count,k);
    base=__shfl_sync(full,base,0);
    unsigned rank=__popc(mask & ((1u<<lane)-1u));
    if(yes){unsigned dst=base+rank;if(dst<capacity)queue[dst]=i;else atomicAdd(dropped,1u);}
}
__global__ void cheap_all(const Vec* x,float* out,unsigned n) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x;
    if(i<n)for(int d=0;d<hidden;++d)out[std::size_t(i)*hidden+d]=cheap(x[i],d);
}
__device__ inline void warp_mlp(const Vec& x,const Weights* w,unsigned original,float* out) {
    unsigned lane=threadIdx.x&31;float h=w->b1[lane];
    for(int d=0;d<channels;++d)h=fmaf(x.x[d],w->w1[lane*channels+d],h);
    h=activate(h);float z=0;
    for(int j=0;j<hidden;++j)z=fmaf(__shfl_sync(full,h,j),w->w2[lane*hidden+j],z);
    out[std::size_t(original)*hidden+lane]=cheap(x,lane)+activate(z);
}
__global__ void refine_queue(const Vec* x,const unsigned* queue,const unsigned* count,
                             unsigned capacity,const Weights* w,float* out) {
    unsigned tid=blockIdx.x*blockDim.x+threadIdx.x,warp=tid>>5;
    unsigned stride=(gridDim.x*blockDim.x)>>5, end=min(*count,capacity);
    // Bounded immutable worklist, NOT a spinning persistent runtime.
    for(unsigned q=warp;q<end;q+=stride){unsigned i=queue[q];warp_mlp(x[i],w,i,out);}
}
__global__ void coarsen_float_records(const float* fine,unsigned fine_n,Vec* coarse,
                                      unsigned* source_ids,unsigned coarse_n) {
    unsigned g=blockIdx.x*blockDim.x+threadIdx.x;if(g>=coarse_n)return;
    unsigned first=4*g,end=min(first+4,fine_n);
    for(int d=0;d<channels;++d){float sum=0;for(unsigned i=first;i<end;++i)sum+=fine[std::size_t(i)*hidden+d];coarse[g].x[d]=sum/float(end-first);}
    // Deliberately non-identity routing makes lineage restoration observable.
    source_ids[g]=coarse_n-1-g;
}
__global__ void cheap_all_sources(const Vec* x,const unsigned* source_ids,float* out,unsigned n) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i<n)for(int d=0;d<hidden;++d)out[std::size_t(source_ids[i])*hidden+d]=cheap(x[i],d);
}
__global__ void refine_queue_sources(const Vec* x,const unsigned* queue,const unsigned* count,
                                    unsigned capacity,const unsigned* source_ids,
                                    const Weights* w,float* out) {
    unsigned tid=blockIdx.x*blockDim.x+threadIdx.x,warp=tid>>5;
    unsigned stride=(gridDim.x*blockDim.x)>>5,end=min(*count,capacity);
    for(unsigned q=warp;q<end;q+=stride){unsigned i=queue[q];warp_mlp(x[i],w,source_ids[i],out);}
}
__global__ void refine_warp_direct(const Vec* x,unsigned n,float threshold,const Weights* w,float* out) {
    unsigned tid=blockIdx.x*blockDim.x+threadIdx.x,warp=tid>>5;
    unsigned stride=(gridDim.x*blockDim.x)>>5;
    for(unsigned i=warp;i<n;i+=stride){Vec v=x[i];if(select(v,threshold))warp_mlp(v,w,i,out);}
}
__global__ void refine_thread_direct(const Vec* x,unsigned n,float threshold,const Weights* w,float* out) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i>=n)return;Vec v=x[i];
    if(!select(v,threshold)){for(int d=0;d<hidden;++d)out[std::size_t(i)*hidden+d]=cheap(v,d);return;}
    float h[hidden];
    for(int j=0;j<hidden;++j){float z=w->b1[j];for(int k=0;k<channels;++k)z=fmaf(v.x[k],w->w1[j*channels+k],z);h[j]=activate(z);}
    for(int d=0;d<hidden;++d){float z=0;for(int j=0;j<hidden;++j)z=fmaf(h[j],w->w2[d*hidden+j],z);out[std::size_t(i)*hidden+d]=cheap(v,d)+activate(z);}
}
struct CUBSelectedIndex {
    const Vec* values; float threshold;
    __host__ __device__ bool operator()(unsigned i) const { return select(values[i],threshold); }
};
// CUB's stable selected index list feeds the same identity-restoring consumer.
inline cudaError_t select_cub(void* temp,std::size_t& temp_bytes,const Vec* x,unsigned n,
                              unsigned* queue,unsigned* count,float threshold,cudaStream_t stream=0) {
    auto indices=thrust::make_counting_iterator<unsigned>(0);
    return cub::DeviceSelect::If(temp,temp_bytes,indices,queue,count,n,CUBSelectedIndex{x,threshold},stream);
}
// E4: equal runtime keys define candidate peer sets, NOT biological equivalence.
__global__ void rendezvous(const Vec* x,const unsigned* keys,Match* out,unsigned n) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x,lane=i&31;bool valid=i<n;
    unsigned active=__ballot_sync(full,valid);
    unsigned peers=__match_any_sync(full,valid?keys[i]:0xffffffffu)&active;
    Vec v=valid?x[i]:Vec{},mean{};unsigned bits=valid?peers:0;
    // All lanes named by peers agree on every source and execute the same calls.
    while(bits){unsigned src=__ffs(bits)-1;
        for(int d=0;d<channels;++d)mean.x[d]+=__shfl_sync(peers,v.x[d],src);
        bits&=bits-1;
    }
    if(valid){for(int d=0;d<channels;++d)mean.x[d]/=float(__popc(peers));out[i]={mean,peers,unsigned(__ffs(peers)-1)};}
}
__global__ void rendezvous_pairwise(const Vec* x,const unsigned* keys,Match* out,unsigned n) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x,lane=i&31;bool valid=i<n;
    unsigned active=__ballot_sync(full,valid),key=valid?keys[i]:0xffffffffu,peers=0;Vec v=valid?x[i]:Vec{},sum{};
    for(unsigned src=0;src<32;++src){unsigned other=__shfl_sync(full,key,src);bool same=valid && ((active>>src)&1u) && other==key;
        if(same)peers|=1u<<src;
        for(int d=0;d<channels;++d){float y=__shfl_sync(full,v.x[d],src);if(same)sum.x[d]+=y;}
    }
    if(valid){for(int d=0;d<channels;++d)sum.x[d]/=float(__popc(peers));out[i]={sum,peers,unsigned(__ffs(peers)-1)};}
}
__global__ void gather_candidates(const Vec* x,const unsigned* keys,const unsigned* order,
                                  Vec* values,unsigned* sorted_keys,unsigned n) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i<n){unsigned src=order[i];values[i]=x[src];sorted_keys[i]=keys[src];}
}
__global__ void scatter_matches(const Match* sorted,const unsigned* order,Match* out,unsigned n) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i<n)out[order[i]]=sorted[i];
}
__global__ void keys_from_means(const Match* x,unsigned* keys,unsigned n) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i<n){unsigned k=0;for(int d=0;d<channels;++d)k|=unsigned(x[i].mean.x[d]>0.f)<<d;keys[i]=k;}
}
__global__ void supertile_all_pairs(const Vec* x,const unsigned* keys,SuperMatch* out,unsigned n) {
    unsigned i=blockIdx.x*blockDim.x+threadIdx.x;if(i>=n)return;SuperMatch m{};m.leader=0xffffffffu;
    for(unsigned j=0;j<n;++j)if(keys[i]==keys[j]){if(m.leader==0xffffffffu)m.leader=j;++m.count;for(int d=0;d<channels;++d)m.mean.x[d]+=x[j].x[d];}
    for(float& v:m.mean.x)v/=float(m.count);out[i]=m;
}
} // namespace bp_cuda_lab
