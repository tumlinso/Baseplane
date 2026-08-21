

<!-- todo-orchestrator:v2-managed:start -->
# CE-BITOP-44: Conditional CelleraTorch bindings

Task revision: `128`; current project revision is in `todo-status.md`.

## Objective
After native interfaces stabilize and activation is explicit, expose event batches/learned parameters as optional Torch-facing views without moving ownership.

## State
- Lifecycle: `blocked`
- Execution: `blocked_dependency`
- Parallel policy: `serial`
- Result: `-`

## Next Action
Remain blocked until native hashes stable and activation set; then work only under Cellerator/components/CelleraTorch.

## Ownership
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `bench`
- `forbidden`: `docs`
- `forbidden`: `include`
- `forbidden`: `src`
- `forbidden`: `tests`

## Dependencies
- `task`: `CE-BITOP-40`
- `task`: `CE-BITOP-41`
- `decision`: `BITOP-CELLERATORCH`
- `decision`: `CELLERATOR-EXTERNAL-STATE`
<!-- todo-orchestrator:v2-managed:end -->
