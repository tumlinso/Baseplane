# Prepared host sequence example

Include `prepared_sequence.hh` from this directory and construct one
`sequence_tool::prepared_sequence` with a forward source snapshot, a five-entry
CE effect vocabulary (A/C/G/T/invalid), and a versioned BP route model.
Call `question(begin, length)`, then `execute(request, detail, detail_capacity,
pairs, pair_capacity)` repeatedly. The focused [consumer](../../../tests/substrate/composition/consumer.cpp)
shows construction, repeated questions, two hierarchy alternatives and edits.

The prepared object owns exact packed sequence/validity, a hierarchy, the global
2-base posting index, incremental values/postings and a prepared GC predicate.
Effects preserve sequence order; the host predicate uses the existing BP GC
kernel. Learned and constructed routes share source identity and explicit
numerical/gradient policy. Learning fits the existing declared synthetic triplet
task, with no biological inference claim. Caller-owned detail/pair buffers may
have zero capacity; full required/stored/dropped counts remain explicit.

Questions use forward owned support. Nonlocal pairs cover the whole prepared
owned source, independently of local question support; use `nonlocal_object(id)`
to resolve their exact source supports. Learned route chunks and inspection cost
also describe the whole snapshot; returned gates are filtered to the local
question. Invalid symbols remain invalid and are omitted from exact detail.
Effects use the caller's explicit invalid transition. Coarse GC/AT carriers are
marked lossy and cannot substitute for exact sequence revisits or certified bounds.

`edited` returns a new snapshot, checks the expected source generation and
returns the incremental owner's invalidation receipt. Equal-length edits repair
incremental values/postings; resize edits rebuild them and advance structure
epoch. The example explicitly rebuilds its derived hierarchy, global index and
routes after every edit. Existing prepared snapshots and their answers retain
original identity. Query execution still scans GC support and encodes learned
routes; it is a composition example, not a new runtime or performance result.

Run the real development SDK consumer:

```sh
python3 tests/substrate/composition/check.py \
  --ce-sdk /tmp/ce-is1-sdk-b --bp-sdk /tmp/bp-is1-sdk-a \
  --ce-core-commit 41952561a1fa9b2aab7d6d563fae2e2006b0850e
```

This requires existing installed CE effects/mechanisms/learning and BP substrate
components. CMake rejects a mismatched installed CE source revision. This does
not qualify CUDA, framework/provider delivery, broad performance, packaging
relocation, arbitrary strands/halo effects, or production readiness.
