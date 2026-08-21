

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-10: Validity, chunk coordinates, and boundary-safe views

Task revision: `128`; current project revision is in `todo-status.md`.

## Objective
Make invalid-base meaning, tail validity, base count, bounded local coordinates, global identity, owned range, and halos explicit without freezing the cross-library ABI.

## State
- Lifecycle: `done`
- Execution: `closed`
- Parallel policy: `parallel_safe`
- Result: `implemented`

## Next Action
Implement only the three owned files; resolve BITOP-CHUNK-COORDINATES; compile/run the standalone test in build-bitop-BP-BITOP-10; hand off provisional declarations to BP-BITOP-13.

## Ownership
- `exclusive`: `include/Baseplane/seq/dna2_validity.hh`
- `exclusive`: `src/seq/dna2_validity.cpp`
- `exclusive`: `tests/seq/test_dna2_validity.cpp`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `include/Baseplane/dna2.hh`
- `forbidden`: `include/Baseplane/seq/dna2.cuh`
- `forbidden`: `include/Baseplane/seq/dna2_scan.cuh`
- `forbidden`: `include/Baseplane/seq/dna2_scan.hh`
- `read`: `include/Baseplane/seq/dna2_ops.hh`
- `read`: `include/Baseplane/seq/dna2_views.hh`

## Dependencies
- `task`: `STACK-BITOP-01`
<!-- todo-orchestrator:v2-managed:end -->
