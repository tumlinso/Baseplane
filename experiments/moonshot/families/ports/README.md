# BP-MOON-120 sequence ports, corrections, factors and worlds

`Baseplane::moonshot_ports` exports `<bp_moon/ports.hpp>` in `bp_moon::ports`.
The family consumes the committed CE-MOON-050 provider through
`Cellerator::moonshot_mechanisms`. There is no copied numerical backend.
Every fixture is bounded to 128 source positions. `Site` retains source ID,
contig, coordinate, source version, strand, local position and exact original base.
Invalid packed positions cannot become valid source support.

| Card | Sequence mechanism | Compared host result |
|---|---|---|
| E41 | Valid boundary bases become two ports. Contiguous valid bases supply synthetic couplings; invalid gaps break adjacency. CE condenses interiors and reconstructs values onto exact source sites. | `ACNTGCA`, reverse strand, origin 1000: six valid sites, ports 1006/1000; reconstruction matches the full CE solve |
| E42 | Observed source-position bins build fine/coarse membership maps over valid sites. CE fits a scalar correction strength and updates the fine states; source support stays attached. | Three bins: residual objective 17.5 -> 0.496539, fitted alpha 1.01591 |
| E43 | Whole-source A/C/G role postings use the preceding exact canonical base as a compatibility key. CE joins/scorers run on these real candidates. Role and context source sites survive expansion. | `TATCTGNACAG`: source roles at 1/3/5 share preceding T, score 14; an N-preceded role is excluded |
| E44 | Sparse base replacements create alternative exact sequences and validity masks. CE executes numerical leaf deltas over an immutable aggregate DAG. | `ACNT` worlds: outputs 7/10/10/3; shared scalar evaluations 13 versus 28 independent, with distinct alternatives retained |

The E41 synthetic matrix has base-dependent positive diagonal values and small
adjacent-valid couplings. Port order is explicit; the model stores all valid-site
source mappings. Boundary responses omit interior state until CE reconstruction
is requested. This reconstructs the specified linear model, not a lossless learned
genome embedding. Exact sequence remains available in the fixture/source.

E42 uses fixed, visible bins of local source positions rather than annotation
labels or an oracle hierarchy. The numerical provider fits one scalar alpha;
restriction/prolongation maps are not learned here. The scalar residual loss is
model-specific and is not an absence certificate. This is one budgeted correction,
not an automatically convergent process. Fine residual-driven selective refinement
and learned maps remain later variants.

E43 discovers candidates from sequence, including the context bases that supplied
their keys. Equal keys describe a deliberately simple relation hypothesis across
the whole fixture; they impose no genomic distance penalty. Counts distinguish
required cardinality, stored tuples and overflow. Capacity zero reports one
required tuple while storing none. `aggregate_factors` retains a factorized
numerical total without expansion; original role postings supply recoverable
provenance. Arbitrary nonlinear factors and biological efficacy are untested.

E44 gives each alternative a unique world ID and preserves the baseline source
identity/version alongside that world. Base replacements may add or remove valid
support. Invalid bases supply zero numerical contribution and no biological
source sites. Equal query groups do not erase exact sequence differences or their
validity masks, and cannot authorize state sharing for a new query. The scalar
response is a toy sum of canonical base codes plus one. These model alternatives
establish no identified causal effects. Evaluation counts exclude state copying,
source extraction and buffer work, so they establish no speedup.

The four mechanisms call distinct CE operators: condensation/backsolve,
coarse correction/parameter fitting, posting joins/factorized reduction and sparse
numerical DAG worlds. The family is host-only; CE's separate sm70 representatives
were compiled, and no GPU run is claimed by this consumer.

```sh
cmake -S experiments/moonshot/families/ports -B /tmp/bp-ports \
  -DCE_MOON_SOURCE_DIR=/explicit/cellerator/experiments/baseplane_moonshot \
  -DCE_MOON_ENABLE_CUDA=OFF
cmake --build /tmp/bp-ports -j1
ctest --test-dir /tmp/bp-ports -R bp_moon_ports_probe -V
```

Campaign builds use `/tmp/moonshot_build_slot.py`. The standalone source path was
the managed CE-MOON-L-MECHANISMS workspace at commit
`6dfc800c8c860a26594aac7778922f4f78e6f722`.
`receipts/consumer.json` binds that provider and records source digests, commands,
card dispositions and limits. `receipts/producer-ce-moon-050.json` preserves the
provider receipt used by this run. The root controller owns final integration and
workflow acceptance.
