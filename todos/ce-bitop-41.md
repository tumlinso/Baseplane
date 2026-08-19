

<!-- todo-orchestrator:v2-managed:start -->
# CE-BITOP-41: Learned sparse event weighting and regional aggregation

Task revision: `16`; current project revision is in `todo-status.md`.

## Objective
In external Cellerator, implement first Torch-free learned layer over sparse Baseplane events and segments using existing parameter_descriptor.

## State
- Lifecycle: `planned`
- Execution: `ready`
- Parallel policy: `serial`
- Result: `-`

## Next Action
Implement under external Cellerator include/src/models/sequence and tests/sequence only after adapter/hash and external guard; acquire model registry lock.

## Ownership
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `bench`
- `forbidden`: `docs`
- `forbidden`: `include`
- `forbidden`: `src`
- `forbidden`: `tests`

## Dependencies
- `task`: `CE-BITOP-40`
- `decision`: `CELLERATOR-EXTERNAL-STATE`
<!-- todo-orchestrator:v2-managed:end -->
