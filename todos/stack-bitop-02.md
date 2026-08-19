

<!-- todo-orchestrator:v2-managed:start -->
# STACK-BITOP-02: Ownership and architecture contract

Task revision: `16`; current project revision is in `todo-status.md`.

## Objective
Create the durable Baseplane-owned BitOp ADR and freeze representation levels, communication, compatibility, validity/coordinates, output policies, compiled-plan meaning, lane-mapping independence, and non-goals.

## State
- Lifecycle: `planned`
- Execution: `ready`
- Parallel policy: `parallel_safe`
- Result: `-`

## Next Action
Author docs/BITOP_ARCHITECTURE.md only, run the pattern/diff gates, reach BITOP_ARCHITECTURE_FROZEN, and hand off the frozen interface hash.

## Ownership
- `exclusive`: `docs/BITOP_ARCHITECTURE.md`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `bench`
- `forbidden`: `include`
- `forbidden`: `src`
- `forbidden`: `tests`
- `read`: `AGENTS.md`
- `read`: `docs/PERFORMANCE_CONTRACT.md`
- `read`: `docs/ROADMAP.md`
- `read`: `docs/SEQUENCE_BITS.md`

## Dependencies
- `task`: `STACK-BITOP-01`
<!-- todo-orchestrator:v2-managed:end -->
