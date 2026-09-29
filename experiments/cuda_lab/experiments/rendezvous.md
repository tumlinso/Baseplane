# Rendezvous: make current information determine who talks


**Hypothesis.** Small current-input routing keys and warp match masks can construct useful candidate neighborhoods without separate cis/trans machinery or a pre-discovered global interaction graph.

**Supplied.** `__match_any_sync` finds equal keys among the 32 presented candidates; peers exchange floats through correctly masked shuffles. The scalar oracle and an all-pairs shuffle kernel implement the same group mean, peer mask and leader. The original values remain separate. Equal keys mean a candidate shared computation, never biological identity.

**Finish the minimum experiment.** Compile and sanitize the kernels on sm_70. Add one bounded online candidate regrouping stage over a small supertile of current representations, using the installed CUB primitive or a small explicit reference. Include its movement/scratch cost. Test a no-regrouping baseline too. Do not build or scan a genome-wide annotation or interaction database before embedding.

The kernel does not search arbitrary distant sequence: it sees only the resident candidate packet. To demonstrate nonlocal relevance, supply distant source IDs in the same runtime candidate set and show how the packet was produced. A packet assembled by a test oracle must be labeled an upper-bound fixture, not a deployed routing solution.

Reuse candidate grouping once at a higher representational level and preserve member-to-source maps. Change placement and packet offsets while keeping actual candidate relationships controlled. A match disappearing at a warp boundary is a boundary effect, not evidence of molecular independence.

**Data.** All keys unique; all keys identical (including UINT_MAX with invalid tail lanes); skewed collisions; random keys; duplicates with very different values; nearby distractors and distant compatible objects. Include deliberately colliding keys and measure harmful grouping and missed useful partners, not just grouping speed.

**Baselines.** All-pairs candidate comparison; local grouping without movement; bounded radix/sort/group alternative when available. All must use the same candidate set and preserve input identities. An exact comparison of the defined key operation is different from accuracy of a biological interpretation.

**Measure.** Key generation, packet formation, match/reduction, scatter, scratch, group-size distribution, candidate recall and collision cost. No claim that arbitrary nonlocal communication is free or that every genome interaction can be represented by an equality key.

**Stop.** Retain as a local co-routing helper if that is all the evidence supports. Reject using equal keys to delete original representations. No global all-to-all, trainable hash-table runtime or new storage system in this epic. N1 establishes instruction support; the biological application remains a hypothesis.

**Receipt.** `results/rendezvous.json` with explicit candidate-generation assumptions and a scope-limited conclusion.
