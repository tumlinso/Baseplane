

<!-- todo-orchestrator:v2-managed:start -->
# STACK-BITOP-51: Multi-GPU chunking and halo ownership

Task revision: `132`; current project revision is in `todo-status.md`.

## Objective
Park multi-GPU sequence integration behind Cellerator's future partition contract; Baseplane remains bounded-chunk, single-device, and stream-explicit.

## State
- Lifecycle: `blocked`
- Execution: `blocked_dependency`
- Parallel policy: `serial`
- Result: `-`

## Next Action
Deferred by user on 2026-09-29 for an intermediary epic. Do not claim, activate, or implement this task. Resume only after explicit user authorization and a replacement BitOP plan has been reviewed and applied.

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
- `decision`: `CELLERATOR-PARTITION-CONTRACT-EXTERNAL`
<!-- todo-orchestrator:v2-managed:end -->
