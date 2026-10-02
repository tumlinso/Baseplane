# Sequence-grounded tensor consumers (BP-MOON-070)

Target `Baseplane::moonshot_tensor`, header `<bp_moon/tensor.hpp>`, namespace
`bp_moon::tensor`. Numerical operators come exclusively from CE-MOON-030
`Cellerator::moonshot_tensor`; provide an explicit `CE_MOON_SOURCE_DIR` to its
isolated source directory. This family adds source meaning and sequence fixtures.

`Window` carries a unique object ID, exact packed sequence and `SourceMap`
(source identity, contig, origin, version, strand, length). Input sequence is
already presented in the stated strand orientation. Coordinate mapping reverses
coordinate order for reverse strand; it does not infer reverse-complement bases.
`Grounding` retains the complete presented sequence, each coordinate and each
validity bit. Exact source data can be revisited after numerical compression.

| Card | Concrete sequence operation | Evidence and limits |
| --- | --- | --- |
| E21 | `pack_windows`, `promote`: tile rows are promoted source windows; four columns are valid A/C/G/T counts. CE applies an explicit feature transform and retains IDs. | `GGCT` yields a transformed two-feature vector `[7,-2]`; distant `AAAT` retains coordinates 10000–10003. `NAC` excludes N from counts but retains its invalid bit and exact string. Counts lose base order; original sequence is available. Transform weights are authored, untrained hypotheses. |
| E22 | `nominate`: directed Q=[G,C], K=[A,T] from exact window counts; excludes self pairs, invalid windows and differing strands. CE compacts scores with explicit capacity. | Source window 10 at 100–103 nominates window 20 at 10000–10003 with score7; unknown window excluded. No distance penalty or candidate discovery claim; callers supply the already nominated cohort. A score is a candidate communication hypothesis, not a causal/regulatory edge. |
| E23 | `chunk_relation`, `concatenate_response`: each valid chunk acts on 16 last-two-base states. CE binary composition computes the response to adjacent chunks in traversal order. | `A` then `CG` matches a direct scalar `ACG` state walk for every input state. Forward and reverse adjacency checked. Incompatible sources/versions/strand or coordinate gaps rejected; invalid bases request refinement rather than invented transitions. Counts here encode paths, not motif positions. |
| E24 | `sample_region`, `query_region`: rows are possible scalar entry states of one region; shared weights depend on its valid GC fraction. CE produces tanh responses and interpolation. | `GGCT` with GC=.75, samples -1,0,1, query .5 returns measured error about .0600214 against the direct provider response. Exact sampled queries have zero interpolation error. Query retains exact sequence/coordinates for revisit. This is an authored nonlinear hypothesis with no learned representation claim. |

Buffers are small host fixture objects, capped at 16 rows. Window counts are
limited to 2²⁴ bases to retain integer FP32 count precision. Relations report
required count and overflow; source objects survive capacity truncation in the
caller cohort. Four features and latent transforms are deliberately lossy; their
source maps support recovery, not proof of a lossless embedding. No discovery
oracle supplies a learned hierarchy. General hierarchy construction and broad
candidate retrieval are outside this family's evidence.

```sh
python /tmp/moonshot_build_slot.py cmake -S experiments/moonshot -B /tmp/bp-moon-tensor-build -DCE_MOON_SOURCE_DIR=/home/tumlinson/Cellerator/.git/todo-orchestrator/0ccaac37-dbbf-448e-a5f8-def197a70aba/workflow-workspaces/ce-moon-l-tensor/experiments/baseplane_moonshot
python /tmp/moonshot_build_slot.py cmake --build /tmp/bp-moon-tensor-build --target bp_moon_tensor_host -j1
ctest --test-dir /tmp/bp-moon-tensor-build -R '^bp_moon_tensor_host$' --output-on-failure
```

Only host sequence consumers are authored/compiled/run/compared here. Provider
sm70 CUDA kernels are separately compiled; no Baseplane CUDA consumer or GPU run
is claimed. No performance benchmark or biological efficacy test was run.
