

<!-- todo-orchestrator:v2-managed:start -->
# Todo Orchestrator v2 Projection

Project revision: `16`

## Workstreams
- `STACK-BITOP-01` | kind: validation_task | status: done | parent: BITOP-00 | objective: Record actual repository relationships, HEADs, branches, upstreams, worktrees, dirty paths, active coordination conflicts, source/API/target state, historical SequenceBits evidence, and published-baseline deviations.
- `STACK-BITOP-02` | kind: workstream | status: planned | parent: BITOP-00 | objective: Create the durable Baseplane-owned BitOp ADR and freeze representation levels, communication, compatibility, validity/coordinates, output policies, compiled-plan meaning, lane-mapping independence, and non-goals.
- `BP-BITOP-03` | kind: validation_task | status: planned | parent: BITOP-00 | objective: Verify current CPU/CUDA semantics and record same-build one-V100 exact count baseline, resource behavior, transfer/output behavior, and regression rule without optimizing.
- `STACK-BITOP-04` | kind: integration_task | status: planned | parent: BITOP-00 | objective: After BITOP-B0 opens, audit the authoritative Baseplane ledger and publish the combined baseline-and-architecture checkpoint without editing implementation or external repository state.
- `BP-BITOP-10` | kind: workstream | status: planned | parent: BITOP-00 | objective: Add optional canonicality sidecar and bounded chunk view semantics without changing canonical-only APIs.
- `BP-BITOP-11` | kind: workstream | status: planned | parent: BITOP-00 | objective: Define direct allowed-mask motif semantics, explicit forward/reverse/both scan modes, invalid-base policy, and palindrome duplicate policy.
- `BP-BITOP-12` | kind: workstream | status: planned | parent: BITOP-00 | objective: Generalize or compose the existing eight-byte motif_hit into a versioned sequence event and explicit count/mask/stable/unordered output descriptors.
- `BP-BITOP-13` | kind: integration_task | status: planned | parent: BITOP-00 | objective: Integrate validity, allowed motifs, and events into the smallest bounded pointer-free portable plan, scalar verifier contract, and interface target wiring.
- `BP-BITOP-20` | kind: workstream | status: planned | parent: BITOP-00 | objective: Implement deterministic scalar oracle for validity-aware exact/allowed/strand/plan/count/mask/event/local grammar semantics.
- `BP-BITOP-21` | kind: workstream | status: planned | parent: BITOP-00 | objective: Implement deterministic host preparation, normalization, family classification, halo/output-density propagation, and scratch query.
- `BP-BITOP-22` | kind: workstream | status: planned | parent: BITOP-00 | objective: Implement CPU/reference boundary-safe mask shifts, bounded relations/windows/count thresholds, run boundaries, and ownership masks with plan-derived halo.
- `BP-BITOP-23` | kind: workstream | status: planned | parent: BITOP-00 | objective: Implement CPU/reference stable count-scan-emit and explicit unordered emission semantics with exact overflow/multiset accounting.
- `BP-BITOP-25` | kind: workstream | status: planned | parent: BITOP-00 | objective: Define pointer-free offset/index segment views and deterministic CPU event-count/popcount/sum/max/emit-if/minimal interval operations.
- `BP-BITOP-24` | kind: validation_task | status: planned | parent: BITOP-00 | objective: Fan in all CPU lanes into deterministic reusable fixtures and authoritative baseplaneBitOpCpuTest wiring.
- `BP-BITOP-30` | kind: workstream | status: planned | parent: BITOP-00 | objective: Preserve M=1 shifted packed scan and implement/measure batched exact mappings that amortize sequence windows.
- `BP-BITOP-31` | kind: workstream | status: planned | parent: BITOP-00 | objective: Implement fused one-shot, explicit resident-plane, capped exact-expansion, warp allowed-motif, and mask-output candidates with validity/strand parity.
- `BP-BITOP-32` | kind: workstream | status: planned | parent: BITOP-00 | objective: Implement boundary-safe CUDA mask grammar using adjacent loads or shared supertiles only where reuse pays, with plan-derived halo and unique anchor ownership.
- `BP-BITOP-33` | kind: workstream | status: planned | parent: BITOP-00 | objective: Implement stable count-scan-emit and unordered ballot/rank/block-reservation emission with caller-owned output/scratch and exact capacity accounting.
- `BP-BITOP-34` | kind: workstream | status: planned | parent: BITOP-00 | objective: Implement only minimum generic segment operations required by the first Cellerator adapter, selecting CUB/custom by measured shape.
- `BP-BITOP-35` | kind: integration_task | status: planned | parent: BITOP-00 | objective: Provide allocation-explicit prepare/query/bind/run surface, integrate all CUDA families, and publish executable Baseplane BitOp v1.
- `CE-BITOP-40` | kind: workstream | status: planned | parent: BITOP-00 | objective: In external Cellerator, create dedicated native sequence-compute adapter binding Baseplane prepared plans to DeviceMathContext with non-owning event/segment views.
- `CE-BITOP-41` | kind: workstream | status: planned | parent: BITOP-00 | objective: In external Cellerator, implement first Torch-free learned layer over sparse Baseplane events and segments using existing parameter_descriptor.
- `CE-BITOP-42` | kind: workstream | status: planned | parent: BITOP-00 | objective: In external Cellerator, keep fixed compute tiles while learning sparse boundary probabilities/region proposals and deterministic compact segments.
- `BP-BITOP-50` | kind: validation_task | status: planned | parent: BITOP-00 | objective: Measure crossover matrix and publish replaceable device-fact-versioned backend policy with same-build correctness checks.
- `STACK-BITOP-51` | kind: validation_task | status: planned | parent: BITOP-00 | objective: In external Cellerator orchestration, validate contig/bounded chunk partitioning, plan-derived halos, local-position rebasing, residency, and identical one/four-device event multisets.
- `STACK-BITOP-52` | kind: integration_task | status: planned | parent: BITOP-00 | objective: Integrate Baseplane public headers/CMake/docs and external Cellerator consumer docs/target while preserving Baseplane::seq, CPU-only, optional CUDA/Highway, and ownership boundaries.
- `STACK-BITOP-53` | kind: integration_task | status: planned | parent: BITOP-00 | objective: Audit actual code, interfaces, resources, projections, leases, optional dispositions, benchmarks, sanitizers, Cellerator independence, and integration readiness without publishing.
- `BP-BITOP-36` | kind: workstream | status: planned | parent: BITOP-00 | objective: If and only if evidence activates it, implement compact associative tile transition composition for a concrete bounded-state pattern.
- `CE-BITOP-43` | kind: workstream | status: planned | parent: BITOP-00 | objective: If activated by a concrete dataset/target/baseline, train continuous motif/grammar proposals and periodically compile discrete Baseplane plans for held-out comparison.
- `CE-BITOP-44` | kind: workstream | status: planned | parent: BITOP-00 | objective: After native interfaces stabilize and activation is explicit, expose event batches/learned parameters as optional Torch-facing views without moving ownership.
- `CE-BITOP-45` | kind: workstream | status: planned | parent: BITOP-00 | objective: If activated by a biological objective, perform sparse retrieval/edge proposal/message passing over event or region objects in Cellerator.
- `CS-BITOP-60` | kind: workstream | status: planned | parent: BITOP-00 | objective: Only after ABI stability and demonstrated cross-process reuse need, let external CellShard wrap an opaque versioned Baseplane/Cellerator-owned image.
- `BITOP-00` | kind: epic | status: blocked | parent: - | objective: Coordinate Baseplane compiled sequence predicates and sparse events through Cellerator learned interpretation and final cross-repository validation without owning implementation files.
<!-- todo-orchestrator:v2-managed:end -->
