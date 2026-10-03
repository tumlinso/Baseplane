# Exact edits and contextual reuse

Base: `21bd3b910add45d357d0b8141197e8f0a3f6b7a7`.

`Baseplane/incremental/reuse.hh` integrates the completed E33/E34/E35 and C03
sequence behaviors into one small ownership/guard boundary. Exact original tokens
are retained, including invalid symbols. Content nomination requires exact token
equality before sharing context-free GC work. Source occurrences, contextual
residuals, source revisions and counterfactual worlds stay distinct from shared
content. Actual `seq::dna2_encode_base_with_validity` and chunk validation provide
the native sequence semantics; no alternative numerical execution engine is added.

`sequence_state` owns a source snapshot, fixed-size chunk GC values and complete
sliding-window posting memberships. Equal-length edits update both affected values
and all crossing-window memberships, including negative/invalid evidence. Edits
return immutable next states and explicit invalidation receipts. Resizes advance
the structure epoch and conservatively rebuild values/memberships. Every edit
advances the mutable value generation. Halo ownership controls posting anchors.
Text is supplied in the declared logical strand; callers own any reverse-complement
conversion, while strand remains part of every guard.

Result, plan and residual stores have separate types and the same full guard rules:
genome/contig/chunk, structure epoch, value generation, origin, length, strand,
occurrence, context generation, model, weights, representation, numerical policy,
world/revision, query and complete inspected support. IDs name caller-declared
complete interpretations; callers must advance them when their semantics change.
Plans store shape data and do not implicitly supply a numerical result.

Cross-generation result reuse requires matching all other guards and a complete
consecutive edit chain outside inspected support. Resizes are conservative misses,
including reverse-coordinate changes. Support includes negative evidence.
World deltas require the complete baseline guard and keep baseline lineage, world
identity/revision and all edit invalidation receipts. Identical-content worlds can
share context-free content but cannot alias guarded results.

Host support covers exact sequence/counts, contextual artifact storage and discrete
edit worlds. Arbitrary learned context, numerical world propagation, compressed
persistence, device execution, scientific counterfactual claims and performance
qualification remain outside this implementation. Existing experimental CE
numerical world providers remain the numerical owner's separate interface.
No unpublished SEQ/HIERARCHY/INDEX source is consumed.

## Native gate

```
python3 tests/substrate/incremental/run_host.py --sanitize
```

Owned gate inputs:
`include/Baseplane/incremental/reuse.hh`,
`tests/substrate/incremental/test_reuse.cpp`,
`tests/substrate/incremental/run_host.py`.
The driver compiles the actual standalone `src/seq/dna2_validity.cpp` dependency
and reports its hash with the owned header/fixture hashes. No CE provider or GPU
is needed. Root owns dependency authority and native gate registration.

Checks: cross-chunk edits invalidate both value chunks and four seam postings;
every single-position valid/invalid substitution matches an independent scalar
directory/count oracle and a full rebuild; insertion/deletion/empty input retain
correct snapshots; hash collisions preserve invalid tokens; all 19 guard dimensions
miss independently across result/plan/residual stores; memo edit chains preserve
outside-support reuse and reject missing/inside/resize chains; derived worlds
preserve lineage and cannot alias baseline results.

Shared build proposal: register `test_reuse.cpp` linked to `Baseplane::seq`.
The existing header install picks up the public API. No root/build changes included.
