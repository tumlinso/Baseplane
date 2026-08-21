

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-20: Scalar CPU reference execution

Task revision: `128`; current project revision is in `todo-status.md`.

## Objective
Implement deterministic scalar oracle for validity-aware exact/allowed/strand/plan/count/mask/event/local grammar semantics.

## State
- Lifecycle: `blocked`
- Execution: `blocked_dependency`
- Parallel policy: `parallel_safe`
- Result: `-`

## Next Action
Implement oracle only in owned files; property-test against unpacked logic; hand off deterministic fixture interface to BP-BITOP-24.

## Ownership
- `exclusive`: `include/Baseplane/seq/predicate_execute.hh`
- `exclusive`: `src/seq/predicate_execute_scalar.cpp`
- `exclusive`: `tests/seq/test_predicate_execute_scalar.cpp`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `include/Baseplane/dna2.hh`
- `forbidden`: `include/Baseplane/seq/dna2.cuh`
- `read`: `include/Baseplane/seq/dna2_allowed.hh`
- `read`: `include/Baseplane/seq/dna2_validity.hh`
- `read`: `include/Baseplane/seq/predicate_plan.hh`
- `read`: `include/Baseplane/seq/sequence_event.hh`

## Dependencies
- `barrier`: `BITOP-B1`
- `interface`: `baseplane-sequence-predicate-v1`
<!-- todo-orchestrator:v2-managed:end -->
