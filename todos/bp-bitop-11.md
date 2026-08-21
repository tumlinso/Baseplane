

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-11: Allowed-base motifs and strand semantics

Task revision: `128`; current project revision is in `todo-status.md`.

## Objective
Define direct allowed-mask motif semantics, explicit forward/reverse/both scan modes, invalid-base policy, and palindrome duplicate policy.

## State
- Lifecycle: `done`
- Execution: `closed`
- Parallel policy: `parallel_safe`
- Result: `implemented`

## Next Action
Implement direct membership and reverse-complement oracle in owned files, with no IUPAC parser; compile/run standalone tests; hand off provisional ABI to BP-BITOP-13.

## Ownership
- `exclusive`: `include/Baseplane/seq/dna2_allowed.hh`
- `exclusive`: `src/seq/dna2_allowed.cpp`
- `exclusive`: `tests/seq/test_dna2_allowed.cpp`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `include/Baseplane/dna2.hh`
- `forbidden`: `include/Baseplane/seq/dna2.cuh`
- `forbidden`: `include/Baseplane/seq/dna2_scan.cuh`
- `forbidden`: `include/Baseplane/seq/dna2_scan.hh`
- `read`: `include/Baseplane/seq/dna2_motif.hh`
- `read`: `include/Baseplane/seq/dna2_ops.hh`

## Dependencies
- `task`: `STACK-BITOP-01`
<!-- todo-orchestrator:v2-managed:end -->
