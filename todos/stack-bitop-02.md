

<!-- todo-orchestrator:v2-managed:start -->
# STACK-BITOP-02: Ownership and architecture contract

Task revision: `128`; current project revision is in `todo-status.md`.

## Objective
Create the durable Baseplane-local BitOp ADR for representation, validity, predicates, outputs, prepared-program meaning, and backend independence without freezing cross-library identity, relation, numerical, runtime, or planner contracts owned by Cellerator.

## State
- Lifecycle: `done`
- Execution: `closed`
- Parallel policy: `parallel_safe`
- Result: `implemented`

## Next Action
Remain blocked until CE-ARCH-10 freezes the shared biological ABI; then author only the Baseplane-local ADR and explicitly import rather than redefine the external identity/operand contract.

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
- `decision`: `CELLERATOR-BIOLOGICAL-ABI-EXTERNAL`
<!-- todo-orchestrator:v2-managed:end -->
