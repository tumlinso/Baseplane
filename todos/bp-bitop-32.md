

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-32: CUDA local grammar, halo, and ownership backend

Task revision: `16`; current project revision is in `todo-status.md`.

## Objective
Implement boundary-safe CUDA mask grammar using adjacent loads or shared supertiles only where reuse pays, with plan-derived halo and unique anchor ownership.

## State
- Lifecycle: `planned`
- Execution: `ready`
- Parallel policy: `parallel_safe`
- Result: `-`

## Next Action
Implement global streaming baseline and conditional shared supertile, test cross-word/block/OOB/duplicate ownership, profile only under leased profiler.

## Ownership
- `exclusive`: `bench/bitop/mask_grammar_bench.cu`
- `exclusive`: `include/Baseplane/seq/mask_grammar.cuh`
- `exclusive`: `src/seq/mask_grammar.cu`
- `exclusive`: `tests/seq/test_mask_grammar_cuda.cu`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `bench/bitop/bitop_bench.cu`
- `forbidden`: `include/Baseplane/dna2.hh`
- `forbidden`: `include/Baseplane/seq/dna2.cuh`
- `read`: `include/Baseplane/seq/mask_grammar.hh`
- `read`: `src/seq/mask_grammar_scalar.cpp`
- `read`: `tests/bitop_fixture.hh`

## Dependencies
- `barrier`: `BITOP-B2`
- `interface`: `baseplane-sequence-predicate-v1`
- `interface`: `baseplane-sequence-event-v1`
- `task`: `BP-BITOP-22`
<!-- todo-orchestrator:v2-managed:end -->
