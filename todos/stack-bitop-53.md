

<!-- todo-orchestrator:v2-managed:start -->
# STACK-BITOP-53: Final cross-repository integration audit

Task revision: `16`; current project revision is in `todo-status.md`.

## Objective
Audit actual code, interfaces, resources, projections, leases, optional dispositions, benchmarks, sanitizers, Cellerator independence, and integration readiness without publishing.

## State
- Lifecycle: `planned`
- Execution: `ready`
- Parallel policy: `project_exclusive`
- Result: `-`

## Next Action
Acquire all integration locks, audit Baseplane and external repos, rerun focused CPU/CUDA/sanitizer/integration evidence, write docs/BITOP_FINAL_AUDIT.md, leave commit/push/pointer publication for separately authorized future work.

## Ownership
- `exclusive`: `docs/BITOP_FINAL_AUDIT.md`
- `forbidden`: `todo-status.md`
- `forbidden`: `todos`
- `forbidden`: `todos.md`
- `read`: `.todo-orchestrator/project.json`
- `read`: `.todo-orchestrator/state.snapshot.json`
- `read`: `AGENTS.md`
- `read`: `CMakeLists.txt`
- `read`: `README.md`
- `read`: `bench`
- `read`: `docs`
- `read`: `include`
- `read`: `src`
- `read`: `tests`

## Dependencies
- `barrier`: `BITOP-FINAL-FANIN`
<!-- todo-orchestrator:v2-managed:end -->
