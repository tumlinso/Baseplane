# Declared sequence-local learned task

Base: `21bd3b910add45d357d0b8141197e8f0a3f6b7a7`.

`Baseplane/learning/sequence_routes.hh` integrates the actual C05/E29 teacher and
hardening provider into a public host representation interface. The declared
synthetic task is: classify each valid source triplet by
`(center GC && previous A) || next T/U`. Labels are this stated sequence-local
objective, never biological annotations or a discovered hierarchy.
The eight training feature states are derived from actual source triplets using
the existing sequence validity encoder. Missing/invalid triplet context emits no
valid gate. Source ownership and halo context remain explicit.

One `encode` interface exposes constructed, soft teacher, discrete, hardened and
residual modes. All modes return source/model provenance, grounded gates and
complete nonuniform capped chunks. Quiet and invalid source positions remain in
the representation. Constructed and learned alternatives consume the same source
features and output types.

The existing CE `ce_moon::learning::fit_truth_table` owns numerical fitting:
independent Bernoulli logit BCE gradients `sigmoid(logit)-target`.
Teacher, discrete threshold route, hardened LUT and residual lookup have separate
version fields and checked parent-version links. Teacher logits support the CE
BCE gradient convention. Discrete selection, hardening, residual overrides and
chunk boundaries stop gradients; no straight-through derivative or end-to-end
chunk-training claim is supplied.

Partial observed training is explicit: unobserved rows keep neutral .5 targets
and zero logits/gradient. Counterexample refinement compares an actual hardened
source result against the declared task, records its source/version/coordinate,
and versions a residual override without mutating teacher/discrete/hardened
artifacts. Corrections are scoped to the declared version-1 feature/task vocabulary.
Model owners must advance versions when changing parameters or interpretations.

Budgeted scheduling reserves one round-robin source fetch regardless of score.
With positive budget every position is served within N calls, including quiet
and invalid-context positions. Zero budget explicitly performs no work. Priority
scores only order remaining work; surprise is not treated as biological function.

## Native gate and actual result

```
python3 tests/substrate/learning/run_host.py --provider-include /home/tumlinson/Cellerator/experiments/baseplane_moonshot/families/learning --sanitize
```

Owned gate inputs:
`include/Baseplane/learning/sequence_routes.hh`,
`tests/substrate/learning/test_learning.cpp`,
`tests/substrate/learning/run_host.py`.
The driver reports hashes for these source inputs and the actual dependencies:
frozen-base `src/seq/dna2_validity.cpp` and the explicit CE `learning.hpp`.
No GPU or Torch training job is launched and no optimizer is reimplemented.

Observed CPU result: total eight-row BCE fell from `8*log(2)=5.54518` to
`0.0817098` after 200 CE fitting epochs. All eight fully observed decisions match
constructed answers; a held-out sequence layout produces 20 matching grounded
gates and bounded nonuniform chunks. This layout test reuses the declared feature
vocabulary and is not unseen-class or biological generalization evidence.
Partial training exposes one genuine unobserved quiet-row error; residual version
25 corrects it while earlier artifact versions remain intact. All 17 quiet source
positions are served in 17 positive-budget exploration epochs. Source invalidity,
artifact-version mismatch and residual revision regressions reject explicitly.

Limits: host synthetic local feature vocabulary, caller-declared objective and
stop-gradient routing. Biological learning, arbitrary learned embeddings/ports,
dense sequence models, CUDA, installed provider exports, calibrated confidence,
asynchronous residency and performance remain unqualified. A combined consumer
with new hierarchy/index/reuse APIs belongs to MERGE-A; this independent leaf
consumes no unpublished leaf source.

Shared build proposal: select existing `Cellerator::moonshot_learning` through the
explicit Baseplane component seam and register this consumer. BUILD owns root
CMake/provider exports. No shared build changes are included.
