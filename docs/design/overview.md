# Baseplane: construct the representation while computing it

## The question

An entire genome is a large sequence, but not every useful question needs equally expensive interaction with every base. Can hierarchical representations make that information accessible at a cost shaped by useful organization rather than raw span alone?

Baseplane's endpoint includes learned floating-point embeddings. Compact DNA and exact bit-level predicates are the grounding layer, not a substitute for learning or the project's complete identity. Exact sequence, coordinates, strand and validity remain available beneath higher representations. An accessible source is not a proof that a lossy summary preserves every task-relevant fact.

## Do not put the answer in a preparatory oracle

Cellerator can discover substantial system structure before compiling repeated execution. Baseplane is trying to infer useful sequence organization during representation. It may compile a vocabulary of transforms, use learned parameters, pack sequence and revisit inputs. It must not assume a completed biological hierarchy and then count only the inexpensive operation on that hierarchy as the cost of discovery.

“Online” here means input-dependent structure is built as part of encoding. It does not forbid multiple passes, exact preprocessing or reuse within a run. The important accounting boundary includes the work that creates routing, candidates, summaries and refinements.

## Locality is an affordance, not a biological verdict

Contiguous sequence is naturally cheap to load and process together. This provides a useful starting organization, without an explicit mandatory genomic-distance weighting or a separate cis/trans model. It does not prove that the important relationship is local. Higher representations may reorganize information, and source coordinates remain recoverable after that reorganization.

A global relation is not free just because its eventual participants fit in a warp. Candidate discovery, movement and missed/colliding matches must be measured. Known annotations may help, but must not become the complete ontology of what can matter.

## Conditional information flow

A conventional branch tree gives different lanes different programs. Baseplane instead experiments with collective operations that turn current decisions into data: masks, compacted records, composable transforms, routing keys or refinement requests. Those data determine the next regular operation. Fewer source-level branches is not itself success; extra arithmetic, registers, movement and unused work can make a branchless formulation slower.

This principle should recur across scales. A physical warp tile is not a biological segmentation rule. Every hierarchical proposal must explain its seam behavior, order sensitivity, source map, summary losses and route for retrieving missing detail.

## The completed CUDA lab is evidence, not the final architecture

The lab tested four ways of mapping information to execution: Boolean evidence into floating summaries (BitLift), composable conditional history (CarryFold), selected objects into cooperative work (Rethread), and runtime keys into peer groups (Rendezvous). The recorded result retained only CarryFold's restricted ordered affine representation as a bounded candidate; CUB was its faster executor. Other fixtures exposed costs or information loss.

That is useful progress: the mechanism, executor and scientific utility are separate things to evaluate. Untrained synthetic fixtures do not establish an organism-scale encoder. The current project should keep the useful substrate and the failed hypotheses legible without freezing them into a universal API.

## Relationship to the rest of the program

Baseplane owns sequence meaning and provenance. It may use Cellerator's numerical execution without abandoning its encoding question. GlassHelix can eventually use sequence-grounded information to constrain hypotheses about dynamics; sequence compatibility alone is not causal evidence. FPGA/streaming realizations remain a longer-term possibility; this cleanup adds no FPGA backend.

See [current capabilities](../status/current.md), [results](../results/index.md) and the [source map](../development/source-map.md).
