

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-13: Bounded predicate-plan and verifier contract

Task revision: `128`; current project revision is in `todo-status.md`.

## Objective
Integrate validity, normalized motifs, and output policies into a bounded pointer-free portable sequence program and prepared lowering aligned with the Cellerator-owned common ABI.

## State
- Lifecycle: `done`
- Execution: `closed`
- Parallel policy: `integration_exclusive`
- Result: `implemented`

## Next Action
Guard shared seams, reconcile three provisional interfaces, define/verify portable ops and limits, wire baseplaneBitOpContractTest, freeze baseplane-sequence-predicate-v1, and hand off exact hash/limits.

## Ownership
- `exclusive`: `CMakeLists.txt`
- `exclusive`: `include/Baseplane/dna2.hh`
- `exclusive`: `include/Baseplane/seq/dna2.cuh`
- `exclusive`: `include/Baseplane/seq/predicate_plan.hh`
- `exclusive`: `src/seq/predicate_plan.cpp`
- `exclusive`: `tests/seq/test_predicate_plan.cpp`
- `read`: `include/Baseplane/seq/dna2_allowed.hh`
- `read`: `include/Baseplane/seq/dna2_validity.hh`
- `read`: `include/Baseplane/seq/sequence_event.hh`

## Dependencies
- `checkpoint`: `BITOP_BASELINE_AND_ARCHITECTURE_READY`
- `task`: `BP-BITOP-10`
- `task`: `BP-BITOP-11`
- `task`: `BP-BITOP-12`
- `decision`: `CELLERATOR-BIOLOGICAL-ABI-EXTERNAL`
<!-- todo-orchestrator:v2-managed:end -->
