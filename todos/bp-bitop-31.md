

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-31: CUDA allowed-motif and plane-predicate backends

Task revision: `128`; current project revision is in `todo-status.md`.

## Objective
Implement fused one-shot, explicit resident-plane, capped exact-expansion, warp allowed-motif, and mask-output candidates with validity/strand parity.

## State
- Lifecycle: `blocked`
- Execution: `blocked_dependency`
- Parallel policy: `parallel_safe`
- Result: `-`

## Next Action
Implement candidates with explicit buffers/scratch; test CPU parity/invalid/strand; collect one-shot versus reuse crossover inputs without silently caching planes.

## Ownership
- `exclusive`: `bench/bitop/allowed_scan_bench.cu`
- `exclusive`: `include/Baseplane/seq/allowed_scan.cuh`
- `exclusive`: `src/seq/allowed_scan.cu`
- `exclusive`: `tests/seq/test_allowed_scan_cuda.cu`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `bench/bitop/bitop_bench.cu`
- `forbidden`: `include/Baseplane/dna2.hh`
- `forbidden`: `include/Baseplane/seq/dna2.cuh`
- `read`: `include/Baseplane/seq/dna2_allowed.hh`
- `read`: `include/Baseplane/seq/dna2_scan.cuh`
- `read`: `include/Baseplane/seq/dna2_validity.hh`
- `read`: `tests/bitop_fixture.hh`

## Dependencies
- `barrier`: `BITOP-B2`
- `interface`: `baseplane-sequence-predicate-v1`
- `interface`: `baseplane-sequence-event-v1`
<!-- todo-orchestrator:v2-managed:end -->
