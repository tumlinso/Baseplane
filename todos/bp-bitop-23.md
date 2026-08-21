

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-23: Deterministic and fast compact emission contracts

Task revision: `128`; current project revision is in `todo-status.md`.

## Objective
Implement CPU/reference stable count-scan-emit and explicit unordered emission semantics with exact overflow/multiset accounting.

## State
- Lifecycle: `blocked`
- Execution: `blocked_dependency`
- Parallel policy: `parallel_safe`
- Result: `-`

## Next Action
Define stable canonical position/predicate order and unordered multiset mode, exact zero-capacity/overflow, and caller scratch contract in owned files.

## Ownership
- `exclusive`: `include/Baseplane/emit/sequence_emit.hh`
- `exclusive`: `src/emit/sequence_emit_scalar.cpp`
- `exclusive`: `tests/emit/test_sequence_emit.cpp`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `include/Baseplane/dna2.hh`
- `forbidden`: `include/Baseplane/seq/dna2.cuh`
- `read`: `include/Baseplane/seq/predicate_plan.hh`
- `read`: `include/Baseplane/seq/sequence_event.hh`

## Dependencies
- `barrier`: `BITOP-B1`
- `interface`: `baseplane-sequence-predicate-v1`
<!-- todo-orchestrator:v2-managed:end -->
