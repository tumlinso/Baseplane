#include "e18_cuda.cuh"
namespace bp_moon::discovery::gpu {
__global__ void kernel(const Row* rows,unsigned row_count,const Query* queries,unsigned* groups) {
    const unsigned q=blockIdx.x,lane=threadIdx.x; // One entire warp per query.
    const auto query=queries[q];unsigned row=query.bucket;
    for(unsigned visited=0;visited<row_count && row<row_count;++visited) {
        const Slot slot=rows[row].slots[lane];
        const unsigned mask=__ballot_sync(0xffffffffu,slot.group!=absent && slot.width==query.width && slot.key==query.key);
        if(mask){const unsigned chosen=unsigned(__ffs(mask)-1);const unsigned group=__shfl_sync(0xffffffffu,slot.group,chosen);if(lane==0)groups[q]=group;return;}
        row=rows[row].next; // Uniform progression; overflow rows cannot drop entries.
    }
    if(lane==0)groups[q]=absent;
}
cudaError_t lookup(const Row* rows,unsigned row_count,const Query* queries,unsigned count,unsigned* groups,cudaStream_t stream) {
    if(!count)return cudaSuccess;
    if(!rows||!queries||!groups||!row_count||count>2147483647u)return cudaErrorInvalidValue;
    kernel<<<count,32,0,stream>>>(rows,row_count,queries,groups);
    return cudaGetLastError(); // Async completion and row publication belong to caller.
}
}
