

<!-- todo-orchestrator:v2-managed:start -->
# STACK-BITOP-51: Multi-GPU chunking and halo ownership

Task revision: `16`; current project revision is in `todo-status.md`.

## Objective
In external Cellerator orchestration, validate contig/bounded chunk partitioning, plan-derived halos, local-position rebasing, residency, and identical one/four-device event multisets.

## State
- Lifecycle: `planned`
- Execution: `ready`
- Parallel policy: `serial`
- Result: `-`

## Next Action
After external ready, implement/validate Cellerator chunk orchestration and acquire gpu-all4 plus benchmark resources only for all-device evidence.

## Ownership
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `bench`
- `forbidden`: `docs`
- `forbidden`: `include`
- `forbidden`: `src`
- `forbidden`: `tests`

## Dependencies
- `task`: `BP-BITOP-35`
- `task`: `BP-BITOP-50`
- `decision`: `CELLERATOR-EXTERNAL-STATE`
<!-- todo-orchestrator:v2-managed:end -->
