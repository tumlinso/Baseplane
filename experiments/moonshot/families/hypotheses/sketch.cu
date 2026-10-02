#include <cuda_runtime.h>
#include <cstddef>
namespace bp_moon_hypotheses_cuda {
// Each descriptor covers an explicit bounded oriented sequence slice. Caller
// validates offsets/lengths against the input capacity. One thread per slice;
// caller-owned output, no allocation/synchronization or interpretation change.
static __device__ unsigned rotate8(unsigned v,unsigned n){n%=8;return ((v<<n)|(v>>((8-n)%8)))&255u;}
__global__ void role_sketch8(const char* sequence,const std::size_t* offsets,
                                  const std::size_t* lengths,unsigned char* output,std::size_t regions){
    const std::size_t id=std::size_t(blockIdx.x)*blockDim.x+threadIdx.x;
    if(id>=regions)return;
    const unsigned codes[5]{0x15,0x37,0x59,0x7b,0xe3},roles[4]{0x01,0x23,0x45,0x67};
    unsigned sketch=0;
    for(std::size_t i=0;i<lengths[id];++i){
        const char c=sequence[offsets[id]+i];unsigned b=4;
        if(c=='A'||c=='a')b=0;else if(c=='C'||c=='c')b=1;else if(c=='G'||c=='g')b=2;else if(c=='T'||c=='t')b=3;
        sketch^=rotate8(codes[b]^roles[i%4],unsigned(i%8));
    }
    output[id]=static_cast<unsigned char>(sketch);
}
} // namespace bp_moon_hypotheses_cuda
