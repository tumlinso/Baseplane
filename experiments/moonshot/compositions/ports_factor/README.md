# C06: regional responses to factors to alternative worlds

The executable `bp_moon_c06_ports_factor` connects three computations:

1. `bp_moon::ports::expose_ports` and `reconstruct_region` turn valid sequence
   sites into condensed regional responses and recovered numerical site values.
2. `discover_factors` supplies observed A/C/G role/context candidates. Their
   reconstructed response values become inputs to Cellerator's factor scorer,
   with explicit weights `[0.25,1,0.5,-0.25]`.
3. Those factor scores become leaves of a Cellerator numerical aggregate DAG.
   Alternative sequences rerun the first two stages; changed scores become sparse
   world deltas, evaluated by `ce_moon::mechanisms::evaluate_worlds`.

The response values flow into the factor scores, and those scores flow into the
world state. A probe checks that scored values differ from the separate
base-code factor fixture. Numerical solves, joins and world execution remain
Cellerator-owned. Baseplane owns candidate construction, exact source/world
carriers and when a changed relation needs rebuilding.

The baseline `TATCTGTATCTGTT` has eight combinations of observed A/C/G roles
sharing preceding T context. A one-record buffer reports eight required records;
explicit refinement retrieves all eight before numerical scoring. Refinement is
bounded to 64 factors, and a 125-factor fixture is rejected rather than silently
truncated. The sequence family bounds source fixtures to 128 positions.

Tail substitutions to invalid bases change the synthetic regional response while
preserving factor roles. Queries in four fixed-topology worlds are approximately
`11.2244, 11.2390, 11.3965, 11.3965`. Two equal-output worlds retain different
exact invalid symbols (`N` versus `X`); neither symbol emits biological support.
The world IDs and exact alternative strings remain separate from numerical query
groups. A fifth alternative invalidates one observed context, reducing the role
relation; it rebuilds an independent DAG rather than applying deltas to the old
factor topology.

The naive comparison independently performs a full Cellerator solve and a small
Cartesian scan of exact compatible roles, then compares the fixture-local score
expression with the routed factor scorer. Each fixed world is also compared with
independent Cellerator DAG execution; the rebuilt world is compared with its full
sequence oracle. Provenance checks cover source ID, contig, source version,
reverse strand, exact coordinates, current exact base and context-base validity.
Duplicate world IDs, duplicate mutation positions and out-of-range positions are
rejected.

There are 36 scalar DAG node evaluations versus 60 in four independent 15-node
DAGs. This count excludes candidate discovery, response reconstruction, reruns,
refinement, state copying and transfer; it establishes no throughput improvement.
The scalar query loses sequence arrangement, while retained source carriers
permit revisiting exact detail. Equal numeric outputs establish no sequence
identity, learned hierarchy or identified causal effects. Learned port bases and
learned restriction/prolongation remain separate variants.

Configure this child directly, or through the aggregate compositions build:

```sh
cmake -S experiments/moonshot/compositions/ports_factor -B /tmp/bp-c06 \
  -DCE_MOON_SOURCE_DIR=/explicit/cellerator/experiments/baseplane_moonshot \
  -DCE_MOON_ENABLE_CUDA=OFF
cmake --build /tmp/bp-c06 --target bp_moon_c06_ports_factor -j1
ctest --test-dir /tmp/bp-c06 -R bp_moon_c06_ports_factor -V
```

Campaign builds were prefixed by `/tmp/moonshot_build_slot.py`. The run consumed
Cellerator integration commit `b72bfa3af4782303bc8630ff355d308b54f55ede`, including
the root's reviewed matrix bounds and nonfinite-score repairs. No GPU was used.
`receipts/c06.json` and adjacent logs record input identities and actual evidence.
