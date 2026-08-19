

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-25: Generic segment and regional-summary primitives

Task revision: `16`; current project revision is in `todo-status.md`.

## Objective
Define pointer-free offset/index segment views and deterministic CPU event-count/popcount/sum/max/emit-if/minimal interval operations.

## State
- Lifecycle: `planned`
- Execution: `ready`
- Parallel policy: `parallel_safe`
- Result: `-`

## Next Action
Implement only generic segment vocabulary and CPU reference; reach BITOP_EVENT_AND_SEGMENT_ABI_READY and freeze baseplane-segment-v1.

## Ownership
- `exclusive`: `include/Baseplane/segment/segment_reduce.hh`
- `exclusive`: `include/Baseplane/segment/segment_view.hh`
- `exclusive`: `src/segment/segment_reduce_scalar.cpp`
- `exclusive`: `tests/segment/test_segment_reduce.cpp`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `include/Baseplane/dna2.hh`
- `forbidden`: `include/Baseplane/seq/dna2.cuh`
- `read`: `include/Baseplane/seq/sequence_event.hh`

## Dependencies
- `barrier`: `BITOP-B1`
- `interface`: `baseplane-sequence-predicate-v1`
<!-- todo-orchestrator:v2-managed:end -->
