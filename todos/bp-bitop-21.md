

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-21: Plan normalization, lowering, and backend classification

Task revision: `218`; current project revision is in `todo-status.md`.

## Objective
Implement deterministic host preparation, normalization, family classification, halo/output-density propagation, and scratch query.

## State
- Lifecycle: `superseded`
- Execution: `closed`
- Parallel policy: `parallel_safe`
- Result: `superseded`

## Next Action
Deferred by user on 2026-09-29 for an intermediary epic. Do not claim, activate, or implement this task. Resume only after explicit user authorization and a replacement BitOP plan has been reviewed and applied.

## Ownership
- `exclusive`: `include/Baseplane/seq/predicate_prepare.hh`
- `exclusive`: `src/seq/predicate_prepare.cpp`
- `exclusive`: `tests/seq/test_predicate_prepare.cpp`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `include/Baseplane/dna2.hh`
- `forbidden`: `include/Baseplane/seq/dna2.cuh`
- `read`: `include/Baseplane/seq/predicate_plan.hh`

## Dependencies
- `barrier`: `BITOP-B1`
- `interface`: `baseplane-sequence-predicate-v1`
<!-- todo-orchestrator:v2-managed:end -->
