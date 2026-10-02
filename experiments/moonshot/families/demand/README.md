# Demand family: question-driven execution

BP-MOON-080 implements four distinct host experiments:

| Card | Mechanism | Disposition |
|---|---|---|
| E25 | Query-pull traversal with exact sequence-derived maximum bounds and replay support | Implemented experimentally |
| E26 | Stable fetch/test opcode cohorts across bounded waves | Implemented experimentally |
| E27 | Discrete priority buckets, persistent debt and reserved exploration | Implemented experimentally |
| E28 | Synchronous local context-tag waves, stable promotion and remote wakeup | Implemented experimentally |

`demand.hpp` contains the host mechanisms, `smoke.cpp` the semantic probes, and
`cuda.cu` two compiled machine paths: uniform GC cohort execution and immutable
previous-wave context-tag propagation. CUDA kernels have not been run. Device
queue compaction, a resident CTA interpreter, learned updates, and full CUDA
parity remain extensions. Ordinary host opcode waves are the implemented E26
representative; no cross-block spinning queue exists.

E25 asks either which valid bases meet a maximum-code threshold or which are GC.
The exact maximum bound is computed from the retained source region and scoped to
source, contig, half-open coordinates and version. It can certify an absent
maximum-threshold answer; it cannot certify a GC answer. Invalid packed payloads
never contribute. Traversal is breadth-first for one question/operator, so depth
cohorts have a natural FIFO order. Child admissions have a pending capacity;
excess regions and exhausted work-budget regions are returned as separate exact
supports for replay. The control root is admitted once even with zero child
capacity. This fixture recomputes bounds by scanning source regions, rather than
claiming free summary construction. Construction/revisit cost is consequently
visible and unmeasured. Maximum certificates are exact; learned numerical
confidence would require a different policy.

E26 carriers retain source identity, contig, coordinates and version. Every input
carrier is checked against the current sequence; externally supplied test payloads
are also validated. Fetch and GC-test cohorts execute separately. A three-wave
probe includes its final empty wave. Admissions beyond capacity remain in
`deferred`; an epoch budget leaves `pending` explicit. `produced`/`stored` count
next-wave transitions, while initial admission deferrals are separate. Selected
carriers preserve exact support and discard unselected work only after the
question's exact predicate is evaluated.

E27 accepts caller-owned value and cost estimates. Fifteen priority buckets favor
high estimated value per cost plus aging debt. One full service per wave is
reserved for round-robin exploration. With a fixed finite population and every
cost fitting the wave budget, every item receives service within N waves. Invalid
zero or unserviceable costs are rejected. Deferred items remain in the caller's
bid vector; debt saturates explicitly and resets upon service. The scheduler
consumes estimates without training a second scorer. Scores express scheduling
preferences, and do not certify biological importance.

E28 uses fixed monotone OR context tags as an executable sequence-facing fixture.
Jacobi rounds read an immutable snapshot, and remote messages include the expected
context version. A changed message wakes its target and adjacent cells; remote
source IDs follow propagation. Two stable visits promote a cell, retaining its
fine support for later wakeup. Capacity-limited frontier visits leave the other
active flags intact. All local cells and their topology remain available; an OR
summary loses arrangement and message magnitudes. Source recovery requires those
retained fine supports and source sequence. These monotone fixture tags do not
establish convergence or stability of a future learned update model. Asynchronous
update semantics are deferred.

Inputs and replay storage are capped by a 4096-base/carrier fixture domain. Query
history retains at most 2*N-1 tree nodes; the pending child capacity is independent
of that history. Local context waves are limited to 64 cells. Output vectors are
bounded by the finite source/carrier domain. CUDA kernels allocate nothing and
use caller-owned, nonaliasing buffers; the caller validates source validity,
carrier domains and buffer capacity. Launch on an explicit stream and complete
one wave before consuming its output. Provenance remains in host sidecars. No
benchmark, GPU launch, biological validation or trained model is claimed.

Provider coordination: CE-MOON-040 supplies `ce_moon::learning::predict_logistic`
for optional future value estimates; E27 currently accepts supplied discrete
values. CE-MOON-050 supplies numerical residual/objective diagnostics, which do
not certify E25 absence. No provider code is duplicated. Exact sequence predicates
and context-tag routing live here; numerical learners/solvers remain in Cellerator.
Catalogue sources S01, S09, S10, S14 and S17 are inspiration only. Code reuses the
existing Baseplane fixture/source map and preserves source version checks.

## Reproduce

```sh
python /tmp/moonshot_build_slot.py cmake -S experiments/moonshot -B /tmp/bp-moon-demand-build \
  -DCE_MOON_SOURCE_DIR=/home/tumlinson/Cellerator/experiments/baseplane_moonshot \
  -DBP_MOON_ENABLE_CUDA=ON \
  -DCMAKE_CUDA_COMPILER=/opt/nvidia/hpc_sdk/Linux_x86_64/26.1/cuda/12.9/bin/nvcc
python /tmp/moonshot_build_slot.py cmake --build /tmp/bp-moon-demand-build \
  --target bp_moon_demand_smoke bp_moon_demand_cuda -j1
ctest --test-dir /tmp/bp-moon-demand-build -R '^bp_moon_demand_smoke$' --output-on-failure
```

`evidence.json` records the exact scope of executed checks. These family-local
sources are excluded from the managed semantic context index; the root authorized
canonical-source inspection for this bounded task.
