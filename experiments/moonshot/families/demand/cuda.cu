#include <cuda_runtime.h>
#include <cstddef>
namespace bp_moon_demand_cuda {
// E26: callers partition opcodes before launch, validate capacity/domain and use
// one stream between waves. No persistent queue and no cross-block spinning.
struct Carrier {unsigned position,code,valid;unsigned long long source,version;};
__global__ void gc_cohort(const Carrier* input,unsigned char* selected,std::size_t count){
 const auto i=std::size_t(blockIdx.x)*blockDim.x+threadIdx.x;
 if(i<count)selected[i]=input[i].valid&&(input[i].code==1||input[i].code==2);
}
// E28: immutable previous-wave input is distinct from next output. Each thread
// owns one local cell, and incoming context was applied before this wave.
// The host retains source supports/message provenance and rejects stale versions.
__global__ void jacobi_tags(const unsigned long long* previous,const unsigned char* active,
                           unsigned long long* next,unsigned char* changed,std::size_t count){
 const auto i=std::size_t(blockIdx.x)*blockDim.x+threadIdx.x;if(i>=count)return;
 auto value=previous[i];if(active[i]){if(i)value|=previous[i-1];if(i+1<count)value|=previous[i+1];}
 next[i]=value;changed[i]=value!=previous[i];
}
} // namespace bp_moon_demand_cuda
