#pragma once
#include "lab.hh"
#include <cuda_runtime.h>
namespace bp_cuda_lab {
// Every launch uses blockDim.x multiple of 32. Kernels never allocate, choose
// devices, synchronize the host or manufacture a stream. Caller owns storage.
constexpr unsigned full=0xffffffffu;
__device__ inline unsigned and3_ptx(unsigned a,unsigned b,unsigned c) {
    unsigned d; asm("lop3.b32 %0, %1, %2, %3, 0x80;" : "=r"(d) : "r"(a),"r"(b),"r"(c));return d;
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
} // namespace bp_cuda_lab
