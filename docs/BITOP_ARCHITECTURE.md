# Baseplane BitOp v1 architecture contract

## Status and imported contract

This document freezes the Baseplane-local BitOp v1 ownership boundary. It
imports, and does not redefine, the Cellerator-owned
`cellerator-biological-abi-v1` interface published by
`CELLERATOR_BIOLOGICAL_ABI_V1_FROZEN`. The reviewed imported interface hash is
`708c359577f347cc2f6540aab378c152d2f67386a563ad47e0a3ed901f2eb272`.
Cellerator remains authoritative for persistent biological identity, hot axis
handles, `SequenceDomain`, heterogeneous operand views, residency vocabulary,
relations, numerical meaning, execution ordering, runtime integration, and
planning policy.

Baseplane records may carry Baseplane-local numeric tokens or bounded local
coordinates. A Cellerator adapter maps them to the shared ABI; Baseplane does
not assign persistent identity widths or interpret Cellerator handles. Changes
to the imported ABI require a new external checkpoint and compatibility review,
not an independent Baseplane definition.

## Ownership

Baseplane owns:

- two-bit packed DNA/RNA storage and split-plane compute views;
- explicit per-base validity, tail masking, bounded local coordinates, owned
  intervals, and halos;
- normalized exact and allowed-base motifs, strand handling, and low-level
  validity-aware Boolean predicates;
- compact mask, count, event, and generic segment output mechanics;
- bounded, pointer-free portable predicate programs and verified prepared
  lowering metadata;
- allocation-free scalar references plus CPU/SIMD/CUDA execution primitives
  with explicit caller-owned storage and streams.

Cellerator owns biological identity and operand compatibility, semantic
relations, float-valued interpretation, fusion decisions, device/runtime
sessions, scratch scheduling, plan selection, learned state, and partition
policy. CellShard owns persistence and storage layout. Baseplane does not depend
on `DeviceMathContext`, `parameter_descriptor`, CellShard, or a dense/SpMM
adapter.

## Representation and validity

Canonical bases use `A=00`, `C=01`, `G=10`, and `T=11`. Packed words are the
storage representation; split planes and one-bit masks are compute
representations. Invalid input is never silently canonicalized to A. An absent
validity sidecar means every base inside the declared count is valid; inactive
tail bits are always invalid.

Every stream view used by BitOp exposes a meaningful base count. Local work is
bounded by `dna2_max_local_base_count`; aggregate/global positions remain
64-bit. Chunk metadata carries global origin, owned interval, and bounded halos.
Events use local 32-bit coordinates, and the Cellerator adapter combines those
coordinates with its authoritative sequence domain. A match anchor is emitted
once by its owning interval even when its dependency window reads halo data.

## Predicate semantics

Exact motifs retain the stable `motif32_exact` meaning. Allowed motifs use four
normalized membership planes, one nonempty set per active position. Invalid
bases mismatch every active motif position. Forward, reverse-complement, and
both-strand evaluation are explicit, as is palindrome deduplication versus
dual reporting. Core v1 parsing accepts normalized masks; IUPAC parsing and
motif-database policy remain outside Baseplane.

Portable predicate programs may compose base equality or membership, GC/base
class/CpG masks, bounded shifts, Boolean mask operations, bounded ANY/ALL,
popcount thresholds, exact or WITHIN spacing, runs/boundaries, event emission,
and generic segment-reduction requests. CPU scalar execution is the semantic
oracle for every accepted v1 operation.

## Output and event contract

Output policy is explicit: count-only, mask materialization, stable compact
emit, or unordered compact emit. Materialization is optional and is not the
integration default. Compact event records use the eight-byte Baseplane core:
32-bit local position, 16-bit predicate/rule identifier, 8-bit payload, and
8-bit flags. Quantitative scores, embeddings, and segment offsets are sidecars.

Every emit result distinguishes logical `total_matches`, physical
`stored_records`, `dropped_records`, and required capacity. Zero capacity is a
valid count-with-overflow request. Stable ordering is coordinate-stable under
the declared predicate ordering; unordered emission promises no physical
order. Callers own buffers and residency, and asynchronous APIs enqueue only on
the supplied stream without hidden allocation, transfer, synchronization, or
truncation.

The compact record and descriptors published by BP-BITOP-12 form
`baseplane-sequence-event-v1`. Their compatibility with Cellerator's event
stream is by explicit adapter/composition: chunk origin, biological axes,
residency, and ownership/halo metadata stay outside each compact event.

## Portable and prepared programs

The portable plan is bounded, versioned, pointer-free, relocatable,
deterministic, and backend-neutral. Verification rejects unknown versions or
operations, invalid references, cycles/forward references, inconsistent
normalized motifs, out-of-range shifts or spans, overflow, and unsupported
resource limits. Its hash covers semantic fields and excludes pointers,
streams, device ordinals, and transient addresses.

Preparation validates first, normalizes derived fields, folds constants,
removes dead operations, computes lookbehind/lookahead and output/scratch
requirements, and classifies a specialized lowering family. A prepared plan is
immutable metadata reusable across compatible chunks. It records requirements
but owns no allocation and performs no transfer or synchronization. CUDA lowers
accepted families to specialized kernels; v1 does not install a general device
bytecode interpreter.

The combined validity, motif, event, portable-plan, and prepared-lowering
surface published by BP-BITOP-13 forms `baseplane-sequence-predicate-v1`.
Hardware lane mapping and tuning remain backend-specific and are not part of
that interface hash.

## Compatibility and non-goals

Existing `Baseplane::seq` names and exact count/emit APIs remain compatible.
Standalone CPU-only Baseplane builds remain supported; CUDA adds explicit
stream operations without changing semantic results. Any ABI revision must be
versioned and revalidated against the scalar oracle and the imported Cellerator
contract.

BitOp v1 does not own FASTA/IUPAC/database parsing, genome ontology, PWM or
floating-point scoring policy, dense per-base embeddings, unrestricted GPU
virtual machines, NVRTC-first execution, dynamic parallelism, Tensor Core
Boolean claims, relation/planner policy, training/autograd, or persistence.
