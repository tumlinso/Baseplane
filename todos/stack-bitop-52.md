

<!-- todo-orchestrator:v2-managed:start -->
# STACK-BITOP-52: Packaging, documentation, and consumer contract

Task revision: `16`; current project revision is in `todo-status.md`.

## Objective
Integrate Baseplane public headers/CMake/docs and external Cellerator consumer docs/target while preserving Baseplane::seq, CPU-only, optional CUDA/Highway, and ownership boundaries.

## State
- Lifecycle: `planned`
- Execution: `ready`
- Parallel policy: `integration_exclusive`
- Result: `-`

## Next Action
Guard all shared Baseplane and external Cellerator seams, reconcile generated docs/targets, validate install/CPU/CUDA/consumer targets; do not create Baseplane::bitop for branding.

## Ownership
- `exclusive`: `CMakeLists.txt`
- `exclusive`: `README.md`
- `exclusive`: `cmake`
- `exclusive`: `docs/BITOP.md`
- `exclusive`: `docs/PERFORMANCE_CONTRACT.md`
- `exclusive`: `docs/ROADMAP.md`
- `exclusive`: `docs/SEQUENCE_BITS.md`
- `exclusive`: `include/Baseplane/dna2.hh`
- `exclusive`: `include/Baseplane/seq/dna2.cuh`
- `read`: `bench`
- `read`: `include`
- `read`: `src`
- `read`: `tests`

## Dependencies
- `barrier`: `BITOP-B3`
- `task`: `CE-BITOP-40`
- `task`: `BP-BITOP-50`
<!-- todo-orchestrator:v2-managed:end -->
