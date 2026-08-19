

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-34: CUDA segment reductions

Task revision: `16`; current project revision is in `todo-status.md`.

## Objective
Implement only minimum generic segment operations required by the first Cellerator adapter, selecting CUB/custom by measured shape.

## State
- Lifecycle: `planned`
- Execution: `ready`
- Parallel policy: `parallel_safe`
- Result: `-`

## Next Action
Agree minimum adapter operations from frozen segment ABI, implement CPU parity/uneven/zero segments/explicit scratch and sm_70-safe kernels.

## Ownership
- `exclusive`: `bench/bitop/segment_reduce_bench.cu`
- `exclusive`: `include/Baseplane/segment/segment_reduce.cuh`
- `exclusive`: `src/segment/segment_reduce.cu`
- `exclusive`: `tests/segment/test_segment_reduce_cuda.cu`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `bench/bitop/bitop_bench.cu`
- `forbidden`: `include/Baseplane/dna2.hh`
- `forbidden`: `include/Baseplane/seq/dna2.cuh`
- `read`: `include/Baseplane/segment/segment_reduce.hh`
- `read`: `include/Baseplane/segment/segment_view.hh`
- `read`: `tests/bitop_fixture.hh`

## Dependencies
- `barrier`: `BITOP-B2`
- `interface`: `baseplane-sequence-predicate-v1`
- `interface`: `baseplane-sequence-event-v1`
- `task`: `BP-BITOP-25`
- `interface`: `baseplane-segment-v1`
<!-- todo-orchestrator:v2-managed:end -->
