

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-36: Conditional finite-state cross-tile composition

Task revision: `128`; current project revision is in `todo-status.md`.

## Objective
If and only if evidence activates it, implement compact associative tile transition composition for a concrete bounded-state pattern.

## State
- Lifecycle: `blocked`
- Execution: `blocked_dependency`
- Parallel policy: `serial`
- Result: `-`

## Next Action
Remain parked until a named pattern proves halos/sparse continuation inefficient, state count bounded, and benchmark supplied.

## Ownership
- `exclusive`: `include/Baseplane/seq/predicate_transition.hh`
- `exclusive`: `src/seq/predicate_transition.cpp`
- `exclusive`: `src/seq/predicate_transition.cu`
- `exclusive`: `tests/seq/test_predicate_transition.cpp`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `include/Baseplane/dna2.hh`
- `read`: `include/Baseplane/seq/mask_grammar.cuh`
- `read`: `include/Baseplane/seq/mask_grammar.hh`

## Dependencies
- `task`: `BP-BITOP-32`
- `decision`: `BITOP-FSM-ACTIVATION`
<!-- todo-orchestrator:v2-managed:end -->
