

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-33: CUDA compact emission

Task revision: `132`; current project revision is in `todo-status.md`.

## Objective
Implement stable count-scan-emit and unordered ballot/rank/block-reservation emission with caller-owned output/scratch and exact capacity accounting.

## State
- Lifecycle: `blocked`
- Execution: `blocked_dependency`
- Parallel policy: `parallel_safe`
- Result: `-`

## Next Action
Deferred by user on 2026-09-29 for an intermediary epic. Do not claim, activate, or implement this task. Resume only after explicit user authorization and a replacement BitOP plan has been reviewed and applied.

## Ownership
- `exclusive`: `bench/bitop/sequence_emit_bench.cu`
- `exclusive`: `include/Baseplane/emit/sequence_emit.cuh`
- `exclusive`: `src/emit/sequence_emit.cu`
- `exclusive`: `tests/emit/test_sequence_emit_cuda.cu`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `bench/bitop/bitop_bench.cu`
- `forbidden`: `include/Baseplane/dna2.hh`
- `forbidden`: `include/Baseplane/seq/dna2.cuh`
- `read`: `include/Baseplane/emit/sequence_emit.hh`
- `read`: `include/Baseplane/seq/sequence_event.hh`
- `read`: `tests/bitop_fixture.hh`

## Dependencies
- `barrier`: `BITOP-B2`
- `interface`: `baseplane-sequence-predicate-v1`
- `interface`: `baseplane-sequence-event-v1`
- `task`: `BP-BITOP-23`
<!-- todo-orchestrator:v2-managed:end -->
