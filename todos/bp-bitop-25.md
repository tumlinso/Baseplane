

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-25: Generic segment and regional-summary primitives

Task revision: `218`; current project revision is in `todo-status.md`.

## Objective
Define pointer-free offset/index segment views and deterministic CPU event-count/popcount/sum/max/emit-if/minimal interval operations.

## State
- Lifecycle: `superseded`
- Execution: `closed`
- Parallel policy: `parallel_safe`
- Result: `superseded`

## Next Action
Deferred by user on 2026-09-29 for an intermediary epic. Do not claim, activate, or implement this task. Resume only after explicit user authorization and a replacement BitOP plan has been reviewed and applied.

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
