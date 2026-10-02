

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-22: Local mask grammar and bounded halo semantics

Task revision: `218`; current project revision is in `todo-status.md`.

## Objective
Implement CPU/reference boundary-safe mask shifts, bounded relations/windows/count thresholds, run boundaries, and ownership masks with plan-derived halo.

## State
- Lifecycle: `superseded`
- Execution: `closed`
- Parallel policy: `parallel_safe`
- Result: `superseded`

## Next Action
Deferred by user on 2026-09-29 for an intermediary epic. Do not claim, activate, or implement this task. Resume only after explicit user authorization and a replacement BitOP plan has been reviewed and applied.

## Ownership
- `exclusive`: `include/Baseplane/seq/mask_grammar.hh`
- `exclusive`: `src/seq/mask_grammar_scalar.cpp`
- `exclusive`: `tests/seq/test_mask_grammar.cpp`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `include/Baseplane/dna2.hh`
- `forbidden`: `include/Baseplane/seq/dna2.cuh`
- `read`: `include/Baseplane/seq/dna2_views.hh`
- `read`: `include/Baseplane/seq/predicate_plan.hh`

## Dependencies
- `barrier`: `BITOP-B1`
- `interface`: `baseplane-sequence-predicate-v1`
<!-- todo-orchestrator:v2-managed:end -->
