

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-03: Preserve and measure current exact baseline

Task revision: `16`; current project revision is in `todo-status.md`.

## Objective
Verify current CPU/CUDA semantics and record same-build one-V100 exact count baseline, resource behavior, transfer/output behavior, and regression rule without optimizing.

## State
- Lifecycle: `planned`
- Execution: `ready`
- Parallel policy: `parallel_safe`
- Result: `-`

## Next Action
Use build-bitop-BP-BITOP-03-cpu and -cuda, run actual current targets, acquire one GPU and exclusive benchmark resource only for the benchmark gate, and record exact toolchain/commands/checksums in docs/bitop_baseline_evidence.md.

## Ownership
- `exclusive`: `docs/bitop_baseline_evidence.md`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `bench`
- `forbidden`: `include`
- `forbidden`: `src`
- `forbidden`: `tests`
- `read`: `CMakeLists.txt`
- `read`: `bench/seq`
- `read`: `docs/PERFORMANCE_CONTRACT.md`
- `read`: `docs/sequence_bits_cuda_profile.md`
- `read`: `include/Baseplane/seq`
- `read`: `src/seq`
- `read`: `tests/seq`

## Dependencies
- `task`: `STACK-BITOP-01`
<!-- todo-orchestrator:v2-managed:end -->
