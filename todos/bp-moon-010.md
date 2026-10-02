

<!-- todo-orchestrator:v2-managed:start -->
# BP-MOON-010: Install a small experimental substrate, not a universal framework

Task revision: `221`; current project revision is in `todo-status.md`.

## Objective
Adopt the reviewed successor policy; preserve exact sequence and completed evidence; install independently buildable seed fixtures, source maps, counters and capability declarations. Unblock experiments without first finishing the entire old BitOp executor. Split mixed seeds by ownership per machine/seed-ownership.json; coordinate with CE-MOON-010, but do not mutate its authority from this task.

## State
- Lifecycle: `done`
- Execution: `closed`
- Parallel policy: `integration_exclusive`
- Result: `implemented`

## Next Action
Read planning/baseplane_moonshot_bootstrap/tasks/BP-MOON-010.md and implement the assigned family, adapting within its scope. Apply the adopted ownership and research policy recorded in planning/baseplane_moonshot_adoption/baseplane-policy.todo-plan.json; verify its authority receipt before seed installation.

## Ownership
- `exclusive`: `experiments/moonshot/CMakeLists.txt`
- `exclusive`: `experiments/moonshot/README.md`
- `exclusive`: `experiments/moonshot/cuda`
- `exclusive`: `experiments/moonshot/host`
- `exclusive`: `experiments/moonshot/include`
- `exclusive`: `experiments/moonshot/python`
- `exclusive`: `planning/baseplane_moonshot_bootstrap/results`
- `forbidden`: `.todo-orchestrator`
- `read`: `experiments/cuda_lab`
- `read`: `experiments/moonshot`
- `read`: `include/Baseplane`
- `read`: `planning/baseplane_moonshot_bootstrap`
- `read`: `src/seq`

## Dependencies
_None._
<!-- todo-orchestrator:v2-managed:end -->
