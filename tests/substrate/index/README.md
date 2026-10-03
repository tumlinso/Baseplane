# Whole-input sequence index

Base: `21bd3b910add45d357d0b8141197e8f0a3f6b7a7`.

`Baseplane/index/sequence_index.hh` derives canonical windows from actual packed
sequence/validity/chunk contracts. Invalid windows never enter the index. Owned
anchors may use halo context. Reverse sources produce reverse-complement windows
and preserve original genomic coordinates. Source identity, structure epoch,
value generation, strand and semantic role survive object construction.
Objects own their exact strings and provenance; input buffers are borrowed only
during construction.

The immutable sorted directory exposes full global equal-key posting ranges.
Directed equal-key pairs have complete algebraic required/stored/dropped counts.
Neither warp width nor the predecessor's 4096-object fixture limit truncates the
input. Host memory remains proportional to extracted windows/postings.

One or two half-window sketches nominate unique unordered pairs. Candidate
capacity counts are complete even when storage is zero. Verification compares
exact canonical windows with an explicit Hamming tolerance; scoring requires
accepted verification and delegates the numeric rule to the caller. The
evaluation-only recall oracle exhaustively compares exact windows, rejects
truncated nomination output, and never creates candidates. Nomination recall,
verification and scores are therefore separately inspectable.

Subscriptions use exact posting ranges with source version/strand/role and
inclusive genomic support filters. Fan-out retains destination and question
role separately from the source role, with complete capacity accounting.
Inclusive endpoints permit supports at the last uint64 coordinate.

Three-role factors use caller-declared compatibility keys and numeric values.
The actual CE `ce_moon::mechanisms::join_factors/aggregate_factors` provider owns
numeric joining/scoring. Ordered role 0/1/2 source supports survive opaque CE IDs.
The factorized aggregate is complete independently of materialization capacity.
Zero output capacity uses the factorized count directly, with no hyperedge walk.
Duplicate postings in a role, foreign role IDs, malformed sources and stale
subscriptions reject explicitly.

Limitations: CPU global bulk construction, host memory, and potentially quadratic
candidate output. Full recall evaluation is quadratic. The existing CE provider
walks the full factor combination space when materializing a nonzero prefix;
a streaming provider improvement belongs to CE. Keys/weights are declarations,
not proof of biological compatibility or causal relevance. No CUDA, asynchronous
resident demand execution, learned auction/wavefront qualification, installed
provider exports or biological/performance result is claimed.

## Native gates and input paths

```
python3 tests/substrate/index/run_host.py --provider-include /home/tumlinson/Cellerator/experiments/baseplane_moonshot/families/mechanisms/include
python3 tests/substrate/index/run_host.py --provider-include /home/tumlinson/Cellerator/experiments/baseplane_moonshot/families/mechanisms/include --sanitize
```

Owned gate input paths:
`include/Baseplane/index/sequence_index.hh`,
`tests/substrate/index/test_index.cpp`,
`tests/substrate/index/run_host.py`.
The driver reports hashes for the header, fixture and actual native dependencies:
the frozen-base `src/seq/dna2_validity.cpp` and explicit CE
`ce_moon/mechanisms.hpp` include. Root owns dependency authority/gate registration.

Tests cover 136-member equal-key ranges crossing warp boundaries, a 5002-object
input beyond the old limit, invalid/tail/halo/reverse source windows, two-probe
deduplication, separate measured recall loss, exact verification and score
rejection, subscription fan-out/staleness, and 9177 factor combinations with
two-record/zero-record capacity and ordered source roles.

Shared build proposal: register an optional header interface through
`baseplane_add_representation_target(... COMPONENTS Cellerator::moonshot_mechanisms)`
and compile this consumer. BUILD owns root CMake and installed CE/export decisions.
No other leaf headers or root edits are required by this patch.
