

<!-- todo-orchestrator:v2-managed:start -->
# Todo Orchestrator v2 Projection

Project revision: `128`

## Workstreams
- `STACK-BITOP-01` | kind: validation_task | status: done | parent: BITOP-00 | objective: Record actual repository relationships, HEADs, branches, upstreams, worktrees, dirty paths, active coordination conflicts, source/API/target state, historical SequenceBits evidence, and published-baseline deviations.
- `STACK-BITOP-02` | kind: workstream | status: done | parent: BITOP-00 | objective: Create the durable Baseplane-local BitOp ADR for representation, validity, predicates, outputs, prepared-program meaning, and backend independence without freezing cross-library identity, relation, numerical, runtime, or planner contracts owned by Cellerator.
- `BP-BITOP-03` | kind: validation_task | status: done | parent: BITOP-00 | objective: Verify current CPU/CUDA semantics and record same-build one-V100 exact count baseline, resource behavior, transfer/output behavior, and regression rule without optimizing.
- `STACK-BITOP-04` | kind: integration_task | status: done | parent: BITOP-00 | objective: After BITOP-B0 opens, audit the authoritative Baseplane ledger and publish the combined baseline-and-architecture checkpoint without editing implementation or external repository state.
- `BP-BITOP-10` | kind: workstream | status: done | parent: BITOP-00 | objective: Make invalid-base meaning, tail validity, base count, bounded local coordinates, global identity, owned range, and halos explicit without freezing the cross-library ABI.
- `BP-BITOP-11` | kind: workstream | status: done | parent: BITOP-00 | objective: Define direct allowed-mask motif semantics, explicit forward/reverse/both scan modes, invalid-base policy, and palindrome duplicate policy.
- `BP-BITOP-12` | kind: workstream | status: done | parent: BITOP-00 | objective: Define Baseplane-local count, mask, stable-emit, and unordered-emit semantics while aligning any public event record with the future Cellerator-owned biological ABI before freeze.
- `BP-BITOP-13` | kind: integration_task | status: done | parent: BITOP-00 | objective: Integrate validity, normalized motifs, and output policies into a bounded pointer-free portable sequence program and prepared lowering aligned with the Cellerator-owned common ABI.
- `BP-BITOP-20` | kind: workstream | status: blocked | parent: BITOP-00 | objective: Implement deterministic scalar oracle for validity-aware exact/allowed/strand/plan/count/mask/event/local grammar semantics.
- `BP-BITOP-21` | kind: workstream | status: blocked | parent: BITOP-00 | objective: Implement deterministic host preparation, normalization, family classification, halo/output-density propagation, and scratch query.
- `BP-BITOP-22` | kind: workstream | status: blocked | parent: BITOP-00 | objective: Implement CPU/reference boundary-safe mask shifts, bounded relations/windows/count thresholds, run boundaries, and ownership masks with plan-derived halo.
- `BP-BITOP-23` | kind: workstream | status: blocked | parent: BITOP-00 | objective: Implement CPU/reference stable count-scan-emit and explicit unordered emission semantics with exact overflow/multiset accounting.
- `BP-BITOP-25` | kind: workstream | status: blocked | parent: BITOP-00 | objective: Define pointer-free offset/index segment views and deterministic CPU event-count/popcount/sum/max/emit-if/minimal interval operations.
- `BP-BITOP-24` | kind: validation_task | status: blocked | parent: BITOP-00 | objective: Fan in all CPU lanes into deterministic reusable fixtures and authoritative baseplaneBitOpCpuTest wiring.
- `BP-BITOP-30` | kind: workstream | status: blocked | parent: BITOP-00 | objective: Preserve M=1 shifted packed scan and implement/measure batched exact mappings that amortize sequence windows.
- `BP-BITOP-31` | kind: workstream | status: blocked | parent: BITOP-00 | objective: Implement fused one-shot, explicit resident-plane, capped exact-expansion, warp allowed-motif, and mask-output candidates with validity/strand parity.
- `BP-BITOP-32` | kind: workstream | status: blocked | parent: BITOP-00 | objective: Implement boundary-safe CUDA mask grammar using adjacent loads or shared supertiles only where reuse pays, with plan-derived halo and unique anchor ownership.
- `BP-BITOP-33` | kind: workstream | status: blocked | parent: BITOP-00 | objective: Implement stable count-scan-emit and unordered ballot/rank/block-reservation emission with caller-owned output/scratch and exact capacity accounting.
- `BP-BITOP-34` | kind: workstream | status: blocked | parent: BITOP-00 | objective: Implement only minimum generic segment operations required by the first Cellerator adapter, selecting CUB/custom by measured shape.
- `BP-BITOP-35` | kind: integration_task | status: blocked | parent: BITOP-00 | objective: Provide allocation-explicit prepare/query/bind/run surface, integrate all CUDA families, and publish executable Baseplane BitOp v1.
- `CE-BITOP-40` | kind: workstream | status: superseded | parent: BITOP-00 | objective: Preserve the historical external-adapter proposal as superseded; authoritative Cellerator integration is CE-ARCH-40 and must not bind Baseplane to DeviceMathContext.
- `CE-BITOP-41` | kind: workstream | status: blocked | parent: BITOP-00 | objective: Park learned sparse event weighting until the Cellerator common ABI, execution order, unified runtime, operation core, and measured planner foundations exist.
- `CE-BITOP-42` | kind: workstream | status: blocked | parent: BITOP-00 | objective: Park learned semantic partitions until the common ABI, runtime, execution-order, operation-core, and real-data planner evidence exist.
- `BP-BITOP-50` | kind: validation_task | status: blocked | parent: BITOP-00 | objective: Measure crossover matrix and publish replaceable device-fact-versioned backend policy with same-build correctness checks.
- `STACK-BITOP-51` | kind: validation_task | status: blocked | parent: BITOP-00 | objective: Park multi-GPU sequence integration behind Cellerator's future partition contract; Baseplane remains bounded-chunk, single-device, and stream-explicit.
- `STACK-BITOP-52` | kind: integration_task | status: blocked | parent: BITOP-00 | objective: Integrate Baseplane-owned public headers, CMake, and documentation against the external Cellerator common-ABI checkpoint while preserving Baseplane::seq, CPU-only, optional CUDA/Highway, and ownership boundaries.
- `STACK-BITOP-53` | kind: integration_task | status: blocked | parent: BITOP-00 | objective: Audit actual code, interfaces, resources, projections, leases, optional dispositions, benchmarks, sanitizers, Cellerator independence, and integration readiness without publishing.
- `BP-BITOP-36` | kind: workstream | status: blocked | parent: BITOP-00 | objective: If and only if evidence activates it, implement compact associative tile transition composition for a concrete bounded-state pattern.
- `CE-BITOP-43` | kind: workstream | status: blocked | parent: BITOP-00 | objective: If activated by a concrete dataset/target/baseline, train continuous motif/grammar proposals and periodically compile discrete Baseplane plans for held-out comparison.
- `CE-BITOP-44` | kind: workstream | status: blocked | parent: BITOP-00 | objective: After native interfaces stabilize and activation is explicit, expose event batches/learned parameters as optional Torch-facing views without moving ownership.
- `CE-BITOP-45` | kind: workstream | status: blocked | parent: BITOP-00 | objective: If activated by a biological objective, perform sparse retrieval/edge proposal/message passing over event or region objects in Cellerator.
- `CS-BITOP-60` | kind: workstream | status: blocked | parent: BITOP-00 | objective: Only after ABI stability and demonstrated cross-process reuse need, let external CellShard wrap an opaque versioned Baseplane/Cellerator-owned image.
- `BITOP-00` | kind: epic | status: blocked | parent: - | objective: Coordinate Baseplane sequence primitives as a subordinate library of the Cellerator biological execution model, preserving standalone CPU/CUDA capability while deferring shared identities, relations, runtime integration, numerical interpretation, fusion, and planning to CE-ARCH-00.
<!-- todo-orchestrator:v2-managed:end -->
