

<!-- todo-orchestrator:v2-managed:start -->
# STACK-BITOP-04: Publish baseline and architecture readiness

Task revision: `128`; current project revision is in `todo-status.md`.

## Objective
After BITOP-B0 opens, audit the authoritative Baseplane ledger and publish the combined baseline-and-architecture checkpoint without editing implementation or external repository state.

## State
- Lifecycle: `done`
- Execution: `closed`
- Parallel policy: `integration_exclusive`
- Result: `validated`

## Next Action
Run the required Baseplane audit gate, verify STACK-BITOP-02 and BP-BITOP-03 evidence, reach BITOP_BASELINE_AND_ARCHITECTURE_READY, and complete validated without claiming implementation work.

## Ownership
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `bench`
- `forbidden`: `include`
- `forbidden`: `src`
- `forbidden`: `tests`
- `read`: `bitop-plan.json`
- `read`: `docs/BITOP_ARCHITECTURE.md`
- `read`: `docs/bitop_baseline_evidence.md`

## Dependencies
- `barrier`: `BITOP-B0`
<!-- todo-orchestrator:v2-managed:end -->
