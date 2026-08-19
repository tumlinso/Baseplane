

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-12: Compact event ABI and output-policy contract

Task revision: `16`; current project revision is in `todo-status.md`.

## Objective
Generalize or compose the existing eight-byte motif_hit into a versioned sequence event and explicit count/mask/stable/unordered output descriptors.

## State
- Lifecycle: `planned`
- Execution: `ready`
- Parallel policy: `parallel_safe`
- Result: `-`

## Next Action
Choose composition/alias versus new record without widening, add descriptors and static assertions/tests, reach BASEPLANE_SEQUENCE_EVENT_V1_FROZEN, and hand off the hash.

## Ownership
- `exclusive`: `include/Baseplane/seq/sequence_event.hh`
- `exclusive`: `tests/seq/test_sequence_event.cpp`
- `forbidden`: `CMakeLists.txt`
- `forbidden`: `include/Baseplane/dna2.hh`
- `forbidden`: `include/Baseplane/seq/dna2.cuh`
- `forbidden`: `include/Baseplane/seq/dna2_scan.cuh`
- `forbidden`: `include/Baseplane/seq/dna2_scan.hh`
- `read`: `include/Baseplane/core/status.hh`
- `read`: `include/Baseplane/seq/dna2_motif.hh`

## Dependencies
- `checkpoint`: `BITOP_BASELINE_AND_ARCHITECTURE_READY`
<!-- todo-orchestrator:v2-managed:end -->
