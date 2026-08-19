

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-30: CUDA packed multi-exact motif backend

Task revision: `16`; current project revision is in `todo-status.md`.

## Objective
Preserve M=1 shifted packed scan and implement/measure batched exact mappings that amortize sequence windows.

## State
- Lifecycle: `planned`
- Execution: `ready`
- Parallel policy: `parallel_safe`
- Result: `-`

## Next Action
Implement candidates A-D in owned files, correctness first, run assigned-GPU tests and leased benchmark gates; preserve M=1 regression rule.

## Ownership
- `exclusive`: `bench/bitop/exact_batch_bench.cu`
- `exclusive`: `include/Baseplane/seq/exact_batch.cuh`
- `exclusive`: `src/seq/exact_batch.cu`
- `exclusive`: `tests/seq/test_exact_batch_cuda.cu`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `bench/bitop/bitop_bench.cu`
- `forbidden`: `include/Baseplane/dna2.hh`
- `forbidden`: `include/Baseplane/seq/dna2.cuh`
- `read`: `include/Baseplane/seq/dna2_scan.cuh`
- `read`: `src/seq/dna2.cu`
- `read`: `tests/bitop_fixture.hh`

## Dependencies
- `barrier`: `BITOP-B2`
- `interface`: `baseplane-sequence-predicate-v1`
- `interface`: `baseplane-sequence-event-v1`
<!-- todo-orchestrator:v2-managed:end -->
