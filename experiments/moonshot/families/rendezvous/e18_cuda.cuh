#pragma once
#include "e18_layout.hpp"
#include <cuda_runtime.h>
namespace bp_moon::discovery::gpu {
// Caller owns device buffers and construction/transfer. Immutable rows must be
// published before lookup on this stream. Result: group or absent; no pair expansion.
cudaError_t lookup(const Row* rows,unsigned row_count,const Query* queries,
                   unsigned count,unsigned* groups,cudaStream_t stream);
}
