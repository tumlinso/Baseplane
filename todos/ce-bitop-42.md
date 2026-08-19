

<!-- todo-orchestrator:v2-managed:start -->
# CE-BITOP-42: Learned semantic partitions

Task revision: `16`; current project revision is in `todo-status.md`.

## Objective
In external Cellerator, keep fixed compute tiles while learning sparse boundary probabilities/region proposals and deterministic compact segments.

## State
- Lifecycle: `planned`
- Execution: `ready`
- Parallel policy: `serial`
- Result: `-`

## Next Action
Implement/validate event-density change points or boundary logits over sparse candidates, compact start/end masks, segment construction and pooling; compare fixed windows.

## Ownership
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `bench`
- `forbidden`: `docs`
- `forbidden`: `include`
- `forbidden`: `src`
- `forbidden`: `tests`

## Dependencies
- `task`: `CE-BITOP-41`
- `decision`: `CELLERATOR-EXTERNAL-STATE`
<!-- todo-orchestrator:v2-managed:end -->
