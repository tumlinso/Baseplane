

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-21: Plan normalization, lowering, and backend classification

Task revision: `128`; current project revision is in `todo-status.md`.

## Objective
Implement deterministic host preparation, normalization, family classification, halo/output-density propagation, and scratch query.

## State
- Lifecycle: `blocked`
- Execution: `blocked_dependency`
- Parallel policy: `parallel_safe`
- Result: `-`

## Next Action
Implement constant folding, mask/shift normalization, deterministic CSE/dead output elimination, exact/allowed specialization, family candidates, halo/density/scratch; do not bake unmeasured thresholds.

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
