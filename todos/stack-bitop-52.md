

<!-- todo-orchestrator:v2-managed:start -->
# STACK-BITOP-52: Packaging, documentation, and consumer contract

Task revision: `218`; current project revision is in `todo-status.md`.

## Objective
Integrate Baseplane-owned public headers, CMake, and documentation against the external Cellerator common-ABI checkpoint while preserving Baseplane::seq, CPU-only, optional CUDA/Highway, and ownership boundaries.

## State
- Lifecycle: `superseded`
- Execution: `closed`
- Parallel policy: `integration_exclusive`
- Result: `superseded`

## Next Action
Deferred by user on 2026-09-29 for an intermediary epic. Do not claim, activate, or implement this task. Resume only after explicit user authorization and a replacement BitOP plan has been reviewed and applied.

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
- `decision`: `CELLERATOR-BIOLOGICAL-ABI-EXTERNAL`
- `task`: `BP-BITOP-50`
<!-- todo-orchestrator:v2-managed:end -->
