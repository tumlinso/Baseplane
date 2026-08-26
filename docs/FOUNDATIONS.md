# Baseplane Scientific Foundations

## Status of this document

This document is Baseplane's scientific constitution and architectural
charter. It defines the project identity, boundaries, hypotheses, aspirations,
open questions, and proof obligations against which later engineering should be
judged.

It is not an implementation roadmap. It does not select an API, data structure,
graph representation, hierarchy, model architecture, accelerator strategy, or
task sequence. Statements are marked by their epistemic role:

- **Implemented today** describes source and contracts that currently exist.
- **Architectural principle** constrains later designs.
- **Scientific hypothesis** is a proposition to test, not an established fact.
- **Long-term aspiration** names a desired capability without promising it.
- **Unresolved question** deliberately leaves a choice open.

## Identity

**Baseplane grounds biology in sequence.**

The base is Baseplane's atomic biological unit. A base is the irreducible unit
of exact sequence identity, variation, provenance, ordering, strand, and
sequence truth. “Atomic” is a statement about grounding, not a mandate that
every useful computation process one nucleotide at a time or use a nucleotide
as its computational token.

Baseplane may eventually reason through motifs, intervals, genes, transcripts,
regulatory structures, haplotypes, alleles, learned features, multiresolution
regions, long-range relationships, or structures not yet named. Its invariant
is explanatory descent:

$$
\text{higher sequence-derived structure}
\longrightarrow
\text{supporting sequence}
\longrightarrow
\text{exact bases},
$$

where scientifically possible. Baseplane should be able to move upward from
bases into useful structure without losing a principled path back down toward
sequence.

This distinguishes the related projects:

- **Baseplane is sequence.**
- **Cellerator is omics.**
- **GlassHelix is dynamics.**
- **CellShard is physical scale, persistence, distribution, and delivery.**

The standing formulation is:

> Baseplane grounds biology in sequence. Cellerator makes biological state
> calculable. GlassHelix models how that state evolves. CellShard makes the
> computation scale.

## Scientific purpose

Baseplane's long-term problem is approximately:

> How can organism-scale genomic sequence be made computationally and
> biologically legible without requiring every biological computation to treat
> the genome as one enormous, uniformly active linear token stream?

Its deepest computational hypothesis is:

$$
\boxed{\text{genomic span need not equal computational context cost}}
$$

if biology contains exploitable organization that allows relevant sequence to
become computationally close while irrelevant sequence remains cheap.

This must be paired immediately with an equal constraint:

$$
\boxed{\text{efficiency must not decide in advance what biology is allowed to matter}}
$$

The hypothesis is unproven. Future work must determine where it holds, what its
true costs are, and whether it applies generally or only to restricted
biological problem classes.

## A working scientific decomposition

The following is a conceptual decomposition, not a mandated software pipeline:

$$
\text{sequence}
\rightarrow
\text{sequence-conditioned possibilities}
\rightarrow
\text{state-conditioned structure}
\rightarrow
\text{realized interactions}
\rightarrow
\text{dynamics}.
$$

Baseplane is primarily concerned with exact sequence and what sequence makes
possible, supports, constrains, or renders incompatible. Cellerator operates
over calculable biological and omic structure and changing values. GlassHelix
reasons about realized state, latent mechanisms, transitions, perturbation
response, trajectories, and dynamics.

These distinctions are scientific safeguards:

- A motif occurrence is not motif activity.
- A possible enhancer is not an active enhancer.
- A physical possibility is not a realized regulatory interaction.
- A sequence-compatible mechanism is not an identified mechanism.

Baseplane may constrain a downstream claim through sequence. It must not turn
sequence compatibility into evidence of realized behavior, and it must not
quietly become a second GlassHelix.

## Genomic organization without a prescribed ontology

The genome participates simultaneously in many overlapping organizations:

- linear sequence;
- genes and transcripts;
- coding structure;
- regulatory relationships;
- haplotypes and alleles;
- structural variation;
- repeats and homology;
- evolutionary relationships;
- three-dimensional organization;
- regulatory programs;
- learned genomic structure;
- state-conditioned relevance.

These organizations need not form one tree. Future Baseplane architecture may
involve overlapping relations, multiresolution representations, learned
geometries, graphs, hierarchical structures, or hybrids. This charter chooses
none of them.

Known biology should be exploited when it is useful and cheap. Current human
annotation must not be treated as the complete ontology of genomic function.
Unknown biology must remain discoverable. A computationally learned sequence
feature should not need to be named “motif,” “enhancer,” “gene,” or any other
existing category before Baseplane can preserve it, relate it to sequence, and
expose it to downstream computation.

## Functional genomic geometry

Linear genomic distance is fundamental sequence information. It is not
necessarily the only useful computational distance.

Two distant loci may be biologically close through regulation, transcript
structure, three-dimensional contact, homology, shared learned structure, or a
relationship not yet recognized. Conversely, nearby sequence may be irrelevant
to a particular biological question.

Baseplane should investigate whether computation can move through biologically
useful geometries while exact linear coordinates and sequence remain
recoverable. This is a research direction, not a selected representation.

## Sequence and state

Sequence is comparatively static, so some genomic information may be computed
once and reused. Possible reusable material includes:

- exact sequence facts;
- sequence-derived structures;
- candidate relationships;
- known genomic organization;
- sequence-conditioned possibilities;
- learned sequence features.

Changing cellular state may select, activate, weight, suppress, or otherwise
condition these possibilities. How much useful organization is genuinely
static remains unresolved.

The central question is:

> Does state mostly select among reusable sequence-conditioned possibilities,
> or must biologically relevant genomic organization frequently be
> reconstructed from state itself?

The answer is central to Baseplane's scaling thesis. Reuse only helps if the
cost and scientific validity of state-conditioned selection compare favorably
with reconstructing the original sequence problem.

## Organism-scale ambition

Baseplane's aspirational domain is genomic scale on the order of

$$
10^9 \text{ bases}.
$$

This is neither an accomplished capability nor a guaranteed universal
requirement. “One billion bases” must not mean merely fitting, indexing,
scanning, or accepting that many bases, nor processing a tiny predefined subset
while calling the full address space context.

The stronger eventual meaning would require:

- relevant sequence anywhere in the declared domain can matter;
- irrelevant sequence remains cheap without being permanently discarded;
- distant relevant regions become computationally close when justified;
- biological state can alter effective relevance;
- exact nucleotide identity remains recoverable;
- overlapping genomic organizations can coexist;
- unknown learned structure remains possible;
- higher-level results retain sequence provenance.

Whether this is generally achievable, or achievable only for restricted
biological questions and regimes, is unresolved.

## Exact and learned sequence computation

Baseplane should not be forced into either extreme: that all biology must be
predefined exact symbolic structure, or that a giant learned sequence model
must rediscover every exact and reusable fact.

Baseplane already has a natural exact domain: base identity, validity, strand,
coordinates, exact predicates, bounded grammar, and compact sequence events.
Future work may also require learned sequence interpretation. Long-context
genomic models, including Evo-like systems, may be collaborators, competitors,
components, or baselines.

Baseplane should ask both:

> What exact or reusable genomic structure should a model not have to relearn
> repeatedly?

and:

> What unknown biology can learned sequence models discover precisely because
> we did not prescribe it?

This charter does not resolve the balance.

## Epistemic discipline and provenance

Baseplane will eventually handle claims with radically different evidence
strength. They must never silently collapse together:

$$
\text{exact sequence fact}
\neq
\text{exact derivation}
\neq
\text{annotation}
\neq
\text{model prediction}
\neq
\text{statistical association}
\neq
\text{perturbational evidence}
\neq
\text{mechanistic inference}.
$$

Provenance and uncertainty are scientific requirements, not administrative
metadata. Higher-level sequence-derived structures should retain enough
provenance to support explanatory descent toward the sequence evidence from
which they arose. This charter does not design that provenance system.

## Alleles and genomic identity

A canonical reference genome is not Baseplane's final worldview. Future
sequence grounding must reason legitimately in the presence of:

- diploidy and phased haplotypes;
- alleles and imprinting;
- structural and copy-number variation;
- alternate sequence;
- somatic mutation and mosaicism;
- comparative genomes.

Reference-coordinate equality is not identical to biological identity or
sequence identity. Resolving that distinction is an architectural problem;
this charter chooses neither a pangenome representation nor a coordinate
system.

## Project boundaries

### Baseplane

Baseplane owns sequence-specific semantics: exact bases, sequence identity,
ordering, strand, alleles and haplotypes, sequence-derived possibility,
sequence-native provenance, and representations or computation whose meaning
remains fundamentally tied to genomic sequence.

### Cellerator

Cellerator owns calculable omic and biological state, general biological
relations, values, transformations, execution geometry, and numerical
computation. A sequence-derived object may cross into Cellerator for execution
without losing its Baseplane provenance.

### GlassHelix

GlassHelix owns dynamics: latent state, mechanisms, trajectories, transitions,
perturbation response, and claims about realized biological behavior.
Baseplane may constrain what sequence can support; it must not convert
plausibility into mechanistic certainty.

### CellShard

CellShard owns persistence, placement, transport, residency, sharding, and
distributed physical realization. It must preserve Baseplane identity and
provenance without acquiring Baseplane's scientific semantics.

## The exact-sequence foundation implemented today

The current repository implements a coherent, independently buildable
exact-sequence substrate:

- compact canonical two-bit DNA/RNA words and split-plane compute views;
- explicit valid/invalid base semantics and inactive-tail masking;
- bounded local coordinates, 64-bit global origins, owned intervals, and halos;
- exact and allowed-base predicates with forward, reverse-complement, both-
  strand, and palindrome-reporting semantics;
- exact and bounded mismatch predicates plus a bounded portable predicate
  grammar;
- count, mask, and compact event/output contracts with explicit capacity and
  overflow accounting;
- pointer-free, versioned, verifiable, backend-neutral predicate descriptions
  and prepared metadata;
- allocation-free exact hot paths with caller-owned storage, declared local
  buffer residency, and CUDA streams;
- scalar reference semantics, optional isolated Highway SIMD, and optional
  CUDA implementations targeting the Volta `sm_70` baseline;
- tests covering representation, validity, coordinates, strands, allowed
  motifs, compact events, and predicate-contract verification.

These are durable exact-sequence foundations. They are not yet the complete
scientific model of Baseplane. In particular, the current bounded predicate
grammar, motif operations, 32-base representations, chunk semantics, and
backend lowerings must not be mistaken for the final architecture.

The bounded predicate program currently has verification, hashing, and
preparation metadata, but no generalized scalar or CUDA executor. Current
blocked execution work is not an implemented capability and is not activated by
this charter.

The exact current contract is documented in
[BITOP_ARCHITECTURE.md](BITOP_ARCHITECTURE.md), with representation and build
details in [SEQUENCE_BITS.md](SEQUENCE_BITS.md) and performance evidence under
the [performance contract](PERFORMANCE_CONTRACT.md).

## Explicitly unresolved architectural questions

Future work must investigate, rather than silently decide:

- explicit versus learned versus hybrid genomic organization;
- whether hierarchy is even the right concept;
- how overlapping genomic geometries coexist;
- how multiresolution reasoning preserves exact sequence grounding;
- how state conditions genomic relevance;
- what can be compiled once and reused;
- what must change with state;
- how unknown learned genomic features remain representable;
- how pangenomic and haplotypic identity should work;
- how exact and quantitative sequence interpretation relate;
- how long-range relationships are represented and traversed;
- how Baseplane interacts with strong long-context sequence models;
- how sequence-derived structures cross into Cellerator;
- what reversibility and provenance require at each abstraction;
- what can be incrementally recomputed after variants;
- which variables, beyond base count, determine true scaling;
- how to prevent combinatorial explosion of derived relations;
- how memory movement and locality shape useful computation;
- when biological organization genuinely helps accelerators;
- when hierarchy or irregular structure harms accelerators;
- which biological questions genuinely need organism-scale context;
- whether roughly one billion functionally legible bases is achievable
  generally or only in restricted regimes.

These questions are not a todo list. They define the problem space within which
future proposals must state assumptions and evidence.

## Proof obligations

Any future architecture claiming the Baseplane thesis should eventually show
that:

- biological organization reduces total relevant computation rather than
  merely moving cost into routing or indexing;
- state-conditioned selection is cheaper than recomputing the original
  sequence problem;
- distant relevant sequence remains reachable;
- unknown biology is not silently excluded;
- nucleotide provenance survives abstraction;
- variant-local reuse is scientifically valid;
- derived combinatorics do not overwhelm sequence savings;
- memory movement is included in performance reasoning;
- learned hierarchy or geometry construction cost is counted;
- sequence grounding can constrain or distinguish higher-level mechanisms
  beyond dynamics alone;
- explicit biological structure beats strong long-context sequence modelling
  in at least some meaningful regimes;
- a billion-base claim represents biological capability rather than address
  space.

These are high-level scientific and engineering obligations, not implementation
tasks. A proposal may be valuable without satisfying all of them, but it must
state which claim it makes and what evidence would falsify it.
