# Baseplane Agent Guide

## Read this first

Baseplane is the sequence-grounding layer of a larger biological-compute
program. Read `docs/FOUNDATIONS.md` before proposing architecture. Read
`docs/BITOP_ARCHITECTURE.md`, `docs/SEQUENCE_BITS.md`, and canonical source
before describing or changing the current implementation.

Use project-control and the repository's coding workflow for live task,
contract, ownership, gate, and repository state. Generated todo projections are
not the authority and must not be edited manually. Do not start blocked work or
convert open scientific questions into an unsolicited task graph.

## Project identity

**Baseplane grounds biology in sequence.**

The base is the atomic biological unit: the irreducible unit of exact sequence
identity, variation, provenance, ordering, strand, and sequence truth. This does
not mean every computation must operate one base at a time or use a nucleotide
as its computational token.

Baseplane may reason through motifs, intervals, genes, transcripts, regulatory
structures, haplotypes, learned features, multiresolution regions, long-range
relationships, or structures not yet named. The invariant is:

```text
higher sequence-derived structure
    -> supporting sequence
    -> exact bases, where scientifically possible
```

Do not mistake current `dna2`, motif, predicate-plan, chunk, or CUDA machinery
for the final definition of Baseplane. It is a durable exact-sequence
foundation beneath a larger scientific purpose.

## Ecosystem boundary

```text
Baseplane  = sequence
Cellerator = omics
GlassHelix = dynamics
CellShard  = physical scale, persistence, distribution, and delivery
```

The compact formulation is:

> Baseplane grounds biology in sequence. Cellerator makes biological state
> calculable. GlassHelix models how that state evolves. CellShard makes the
> computation scale.

Baseplane owns sequence-specific semantics: exact bases, sequence identity,
ordering, strand, alleles and haplotypes, sequence-derived possibility,
sequence-native provenance, and computation whose meaning remains tied to
genomic sequence.

Cellerator owns calculable biological and omic state, general biological
relations, values, transformations, execution geometry, and numerical
computation. Sequence-derived objects may cross into Cellerator without losing
Baseplane provenance.

GlassHelix owns latent state, realized mechanisms, trajectories, transitions,
perturbation response, and dynamics. A motif occurrence is not motif activity;
a sequence-compatible mechanism is not an identified mechanism.

CellShard owns persistence, placement, transport, residency, sharding, and
distributed physical realization. It preserves Baseplane identity and
provenance without acquiring Baseplane semantics.

## Scientific discipline

Treat this as a hypothesis to test, not a proven claim:

```text
genomic span need not equal computational context cost
```

Pair it with the constraint:

```text
efficiency must not decide in advance what biology is allowed to matter
```

Known biology may be used when useful and cheap. Human annotation is not a
complete ontology of genomic function. Unknown learned sequence features must
be preservable and traceable without first receiving a familiar biological
name.

Do not silently collapse evidence classes:

```text
exact sequence fact
!= exact derivation
!= annotation
!= model prediction
!= statistical association
!= perturbational evidence
!= mechanistic inference
```

Provenance and uncertainty are scientific requirements. Higher-level
sequence-derived results should support explanatory descent toward their
sequence evidence.

Linear genomic coordinates are fundamental and must remain recoverable, but
linear distance need not be the only computational distance. Do not choose a
single hierarchy, graph, geometry, coordinate system, pangenome representation,
or model architecture without an explicitly authorized, evidence-bearing task.

## Current implemented foundation

Canonical source currently provides:

- two-bit DNA/RNA packed words and split-plane compute views;
- explicit valid/invalid base semantics and inactive-tail masking;
- bounded local coordinates, global origins, owned intervals, and halos;
- exact and allowed-base predicates with explicit strand and palindrome policy;
- count, mask, and compact event/output contracts with explicit overflow;
- bounded, pointer-free, versioned, backend-neutral predicate programs and
  prepared metadata;
- caller-owned storage, declared local buffer residency, and CUDA streams in
  current exact hot paths;
- a scalar reference, optional isolated Highway SIMD, and optional CUDA;
- Volta `sm_70` as the current native CUDA tuning baseline.

The current public library target is `Baseplane::seq`. CPU-only builds remain a
required capability. Current exact count and emit APIs and frozen sequence
predicate/event contracts must be treated as compatibility surfaces unless a
separately authorized revision changes them.

This foundation does not establish a general organism-scale context mechanism,
state-conditioned genomic organization, learned genomic geometry, pangenomic
identity, or dynamics model. Never imply otherwise.

The bounded predicate program currently implements verification, hashing, and
preparation metadata, not a generalized scalar or CUDA executor. Consult live
project-control state before describing any later execution work.

## Engineering laws for authorized implementation work

These laws apply when implementation work is explicitly authorized. They do
not themselves authorize work.

### Preserve sequence meaning

- Document the biological unit represented by thread, lane, warp, block, grid,
  segment, or record.
- Preserve exact coordinates, strand, validity, ownership, and provenance.
- Make ambiguity and invalidity explicit; never infer biological meaning from
  an invalid two-bit payload.
- Keep scalar/reference semantics authoritative for exact operations.

### Match representation to operation

- Packed words are current storage and shifted-window scan representations.
- Split planes and masks are current predicate-compute representations.
- Compact records are current sparse output representations.
- These are mechanisms, not commitments for all future sequence structure.

### Keep costs visible

- No hidden allocation, transfer, synchronization, or dense materialization in
  hot paths.
- Callers own buffers, capacities, residency, streams, and scratch.
- Count host preparation, H2D, kernel, D2H, indexing, routing, learned-structure
  construction, and end-to-end time separately where relevant.
- A scaling claim must count memory movement and derived combinatorics.

### Keep boundaries intact

- Do not add Cellerator state semantics, numerical meaning, planning, or
  partition policy to Baseplane.
- Do not add GlassHelix mechanism or dynamics claims to Baseplane.
- Do not add CellShard persistence or distributed delivery ownership to
  Baseplane.
- Do not add parsers, genome databases, annotation ontology, storage formats,
  training loops, or workflow policy merely because sequence uses them.

### Preserve backend discipline

- CUDA is optional; CPU scalar correctness is mandatory.
- Backend-specific types stay out of backend-neutral public contracts unless a
  reviewed interface explicitly requires them.
- Do not require post-Volta features for the baseline path.
- Do not use accelerator techniques without a biological mapping and measured
  benefit.

### Preserve sparse and explicit output semantics

- Prefer count, compact event, or justified mask output over dense per-window
  materialization.
- Report capacity, stored records, dropped records, required capacity, and
  ordering guarantees explicitly.
- Never silently truncate.

## Validation and evidence

Describe exactly what was inspected, built, tested, benchmarked, or left
unvalidated. Historical benchmark documents are provenance for their recorded
source and toolchain, not current performance proof.

For implementation changes, compare CPU/reference and accelerator behavior on
random inputs, boundary lengths, validity tails, word/chunk boundaries,
reverse complements, strand policy, output capacity, and empty cases as
applicable. Performance runs must record hardware, toolchain, build flags,
inputs, hit density, output mode, memory movement, kernel time, end-to-end time,
and resource use. Use the repository benchmark mutex and live coordination
resources.

## Decision rule

When deciding whether work belongs in Baseplane, ask:

> Is its scientific meaning fundamentally tied to genomic sequence, and can
> its higher-level result retain a principled path to supporting sequence and
> exact bases?

If not, it likely belongs in Cellerator, GlassHelix, CellShard, or another
layer. If the answer is uncertain, preserve the question rather than freezing
an architecture prematurely.
