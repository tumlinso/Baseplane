# Alternative sequence interpretations

The four host probes are in `hypotheses.hpp` and `smoke.cpp`. Link
`Baseplane::moonshot_hypotheses`; Cellerator's learning provider supplies actual
logistic fitting and scoring through `Cellerator::moonshot_learning`. Configure
the existing moonshot build with an explicit `CE_MOON_SOURCE_DIR` containing
that provider. No numerical learner is duplicated here.

E37 stores immutable parse nodes with shared prefixes, exact source references,
segment supports and alternative boundaries. Two CE-fitted synthetic query
scorers prefer different interpretations of the same eight bases. The bounded
beam reports retained and dropped alternatives; pruning can lose a later query's
preferred branch. Scores are hypotheses, not posterior probabilities or learned
biological segmentation. The alternatives are supplied as an ambiguity fixture;
no learned whole-genome hierarchy is claimed.

E38 rotates position/role-bound codes into a deliberately small eight-bit sketch.
AC and CA differ, but a constructed collision nominates different exact strings.
Exact verification replays both source slices and rejects that candidate.
The sketch is lossy nomination machinery. All source bytes and validity remain
available. The optional sm_70 kernel authors the same per-region sketch with
caller-owned offsets, lengths and output; it is compiled without a GPU launch.

E39 uses a typed portfolio: canonical-base counts answer abundance; a finite-state
sequence effect answers terminal-state propagation; unsupported exact-motif
questions replay the original valid bases. AC and CA agree under counts and
differ under ordered effects. Source support is retained with contig, absolute
coordinates, strand and epoch. Invalid bases are never treated as canonical DNA.
This is question-specific sufficiency, not a universal learned representation.

E40 detects a histogram collision with different terminal-state answers and adds
an effect residual only at those two failure sites. It returns a new index while
the previous index and original sequence remain intact. Other sites still replay
source. Seeded random exploration can nominate sites beyond current failures;
held-out repair performance is not measured. The retained scalar effect does not
make all order-sensitive questions sufficient; a different question can still
require replay. The source pointers enable replay rather than lossless embedding.

The host fixtures own their containers, with beam capacity and drops explicit.
Parse extension validates support bounds and source identity; coordinate overflow
is rejected. Reverse strand is metadata for caller-supplied oriented sequence;
the experiment does not silently reverse or complement the sequence. No benchmark,
GPU correctness, biological efficacy or production-interface claim is made.
