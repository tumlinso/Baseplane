# C04: real nonlocal context

This connected host experiment combines E14, E17, E22 and E28. Two distant
regions of one source genome contain `GATCGATC`; source ID, contig, version and
strand agree, while origins are 100 and 10000. No candidate keys or biological
annotations are supplied. `nonlocal_context.hpp` exposes `run`, `revisit`, and
small sequence boundary adapters; the executable is `bp_moon_nonlocal_context`.

1. Fit a contextual boundary scorer through CE `fit_logistic` on a bounded
   synthetic training sequence with a positive-boundary budget penalty. Inference
   sees adjacent GC classes and validity, then predicts through CE
   `predict_logistic`; training labels are absent from the inference path.
   Hierarchy `chunks` and `inverse_map` ground inferred regions in exact bases.
2. Discovery `extract` visits every valid four-base window across complete input
   regions, including windows spanning inferred boundaries. `Directory` and
   `tiled_pairs` build input-derived exact postings and directed candidates.
3. Tensor `pack_windows` creates object-by-base-count rows. CE `relation_scores`
   computes directed Q=[G,C], K=[A,T] scores. The mask contains only actual
   directory candidates between distinct input regions on the same strand.
   CE `compact_relations` emits positive-scoring pairs with capacity/overflow.
4. The inferred inverse map routes each destination object's complete interval
   to affected chunk cells. Demand `wake` delivers contextual tags and source
   lineage; `jacobi` executes bounded update waves. Each input region has a
   separate cell vector, so array-neighbor propagation never invents a bridge
   between distant regions or contigs. Selected relations provide that bridge.
5. Every retained edge carries both complete exact revisits, original source
   metadata, object IDs, endpoint coordinates, score and destination chunk IDs.
   Revisit checks source version, validity, canonical payload and exact oriented
   coordinates before returning the original presented substring.

The fixture reports 8 inferred chunks, 10 extracted objects, 18 exact directory
pairs, 14 scored inter-region edges, 8 update waves, 16 updates, 24 deferred
update visits and 28 exact endpoint revisits. Independent all-pairs equality
checks directory output; an independent count arithmetic oracle checks scores.
Checks also cover inferred-boundary crossing, source reconstruction, actual fit
objective improvement, sparse lineage/support, stale versions, changed
validity and malformed empty object support. A one-edge/one-wave run reports overflow and unfinished convergence.
Invalid windows never enter candidate discovery.

The numerical owners are the real CE tensor and learning providers. This module
adds sequence adapters, not numerical kernels. Its contextual update is the
existing fixed bit-tag Jacobi hypothesis, and its roles are authored count maps.
Exact k-mer equality is not a regulatory relation, inferred boundaries are not
biological entities, and this toy fit does not establish a learned genome-wide
hierarchy. Four-base keys lose wider context; counts lose order; original source
remains available for revisit. No biological efficacy or throughput is claimed.

The experiment caps extracted objects at sixteen and rejects larger inputs
rather than truncating discovery. Each source region is bounded by the demand
fixture and must fit at most 64 chunk cells. Full directory output fits within
256 pair slots for this domain. Relation output has caller capacity and explicit
required/overflow; update capacity and a maximum 64-wave budget leave active
cells visible when incomplete. This is a CPU composition; GPU execution,
large-scale streaming directory-to-tile scheduling and performance accounting
remain later qualification.

```sh
python /tmp/moonshot_build_slot.py cmake -S experiments/moonshot/compositions -B /tmp/bp-moon-c04-build -DCE_MOON_SOURCE_DIR=/home/tumlinson/Cellerator/.git/todo-orchestrator/0ccaac37-dbbf-448e-a5f8-def197a70aba/workflow-workspaces/ce-moon-l-integration/experiments/baseplane_moonshot
python /tmp/moonshot_build_slot.py cmake --build /tmp/bp-moon-c04-build --target bp_moon_nonlocal_context -j1
ctest --test-dir /tmp/bp-moon-c04-build -R '^bp_moon_nonlocal_context$' --output-on-failure
```
