

<!-- todo-orchestrator:v2-managed:start -->
# Todo Status v2 Projection

Project revision: `16`

## Workstreams
- `STACK-BITOP-01` | status: done | execution: closed | next: Verify the recorded evidence against current Git/source without changing implementation; run context, changes, guard, and audit; then complete validated.
- `STACK-BITOP-02` | status: planned | execution: ready | next: Author docs/BITOP_ARCHITECTURE.md only, run the pattern/diff gates, reach BITOP_ARCHITECTURE_FROZEN, and hand off the frozen interface hash.
- `BP-BITOP-03` | status: planned | execution: ready | next: Use build-bitop-BP-BITOP-03-cpu and -cuda, run actual current targets, acquire one GPU and exclusive benchmark resource only for the benchmark gate, and record exact toolchain/commands/checksums in docs/bitop_baseline_evidence.md.
- `STACK-BITOP-04` | status: planned | execution: ready | next: Run the required Baseplane audit gate, verify STACK-BITOP-02 and BP-BITOP-03 evidence, reach BITOP_BASELINE_AND_ARCHITECTURE_READY, and complete validated without claiming implementation work.
- `BP-BITOP-10` | status: planned | execution: ready | next: Implement only the three owned files; resolve BITOP-CHUNK-COORDINATES; compile/run the standalone test in build-bitop-BP-BITOP-10; hand off provisional declarations to BP-BITOP-13.
- `BP-BITOP-11` | status: planned | execution: ready | next: Implement direct membership and reverse-complement oracle in owned files, with no IUPAC parser; compile/run standalone tests; hand off provisional ABI to BP-BITOP-13.
- `BP-BITOP-12` | status: planned | execution: ready | next: Choose composition/alias versus new record without widening, add descriptors and static assertions/tests, reach BASEPLANE_SEQUENCE_EVENT_V1_FROZEN, and hand off the hash.
- `BP-BITOP-13` | status: planned | execution: ready | next: Guard shared seams, reconcile three provisional interfaces, define/verify portable ops and limits, wire baseplaneBitOpContractTest, freeze baseplane-sequence-predicate-v1, and hand off exact hash/limits.
- `BP-BITOP-20` | status: planned | execution: ready | next: Implement oracle only in owned files; property-test against unpacked logic; hand off deterministic fixture interface to BP-BITOP-24.
- `BP-BITOP-21` | status: planned | execution: ready | next: Implement constant folding, mask/shift normalization, deterministic CSE/dead output elimination, exact/allowed specialization, family candidates, halo/density/scratch; do not bake unmeasured thresholds.
- `BP-BITOP-22` | status: planned | execution: ready | next: Implement correct repeated-shift baseline and ownership mask, test exhaustive boundaries/radii, hand off halo convention to CUDA lane.
- `BP-BITOP-23` | status: planned | execution: ready | next: Define stable canonical position/predicate order and unordered multiset mode, exact zero-capacity/overflow, and caller scratch contract in owned files.
- `BP-BITOP-25` | status: planned | execution: ready | next: Implement only generic segment vocabulary and CPU reference; reach BITOP_EVENT_AND_SEGMENT_ABI_READY and freeze baseplane-segment-v1.
- `BP-BITOP-24` | status: planned | execution: ready | next: Guard CMake, build deterministic generators/fixture schema, wire baseplaneBitOpCpuTest, run CPU-only acceptance, reach BITOP_CPU_REFERENCE_READY.
- `BP-BITOP-30` | status: planned | execution: ready | next: Implement candidates A-D in owned files, correctness first, run assigned-GPU tests and leased benchmark gates; preserve M=1 regression rule.
- `BP-BITOP-31` | status: planned | execution: ready | next: Implement candidates with explicit buffers/scratch; test CPU parity/invalid/strand; collect one-shot versus reuse crossover inputs without silently caching planes.
- `BP-BITOP-32` | status: planned | execution: ready | next: Implement global streaming baseline and conditional shared supertile, test cross-word/block/OOB/duplicate ownership, profile only under leased profiler.
- `BP-BITOP-33` | status: planned | execution: ready | next: Implement both explicit policies, CPU multiset parity, stable byte determinism, sparse-to-dense density cases, then run memcheck/racecheck with exclusive sanitizer resource.
- `BP-BITOP-34` | status: planned | execution: ready | next: Agree minimum adapter operations from frozen segment ABI, implement CPU parity/uneven/zero segments/explicit scratch and sm_70-safe kernels.
- `BP-BITOP-35` | status: planned | execution: ready | next: Guard shared seams, integrate CUDA lanes, wire baseplaneBitOpCudaTest, expose required_scratch/prepare/bind/run and inspectable backend choice, run CPU/CUDA/sanitizer fan-in, reach both executable checkpoints.
- `CE-BITOP-40` | status: planned | execution: ready | next: Only after external decision ready: work under /home/tumlinson/CellStack/Cellerator; re-read its guides; guard external claims; use include/Cellerator/compute/sequence, src/compute/sequence, tests/sequence and minimal CMake seam.
- `CE-BITOP-41` | status: planned | execution: ready | next: Implement under external Cellerator include/src/models/sequence and tests/sequence only after adapter/hash and external guard; acquire model registry lock.
- `CE-BITOP-42` | status: planned | execution: ready | next: Implement/validate event-density change points or boundary logits over sparse candidates, compact start/end masks, segment construction and pooling; compare fixed windows.
- `BP-BITOP-50` | status: planned | execution: ready | next: Guard shared benchmark/CMake seams, wire baseplaneBitOpBench, run serialized matrix/checksum/sanitizer/nsys evidence, set measured policy decisions, publish selector inputs and exact commands.
- `STACK-BITOP-51` | status: planned | execution: ready | next: After external ready, implement/validate Cellerator chunk orchestration and acquire gpu-all4 plus benchmark resources only for all-device evidence.
- `STACK-BITOP-52` | status: planned | execution: ready | next: Guard all shared Baseplane and external Cellerator seams, reconcile generated docs/targets, validate install/CPU/CUDA/consumer targets; do not create Baseplane::bitop for branding.
- `STACK-BITOP-53` | status: planned | execution: ready | next: Acquire all integration locks, audit Baseplane and external repos, rerun focused CPU/CUDA/sanitizer/integration evidence, write docs/BITOP_FINAL_AUDIT.md, leave commit/push/pointer publication for separately authorized future work.
- `BP-BITOP-36` | status: planned | execution: ready | next: Remain parked until a named pattern proves halos/sparse continuation inefficient, state count bounded, and benchmark supplied.
- `CE-BITOP-43` | status: planned | execution: ready | next: Remain parked until prediction target, dataset, baseline, seed plan, and resource budget are recorded.
- `CE-BITOP-44` | status: planned | execution: ready | next: Remain blocked until native hashes stable and activation set; then work only under Cellerator/components/CelleraTorch.
- `CE-BITOP-45` | status: planned | execution: ready | next: Remain parked until concrete biological objective, edge source, baseline and sparse complexity budget exist.
- `CS-BITOP-60` | status: planned | execution: ready | next: Remain parked until stable ABI, demonstrated reuse/deployment need, regeneration-cost evidence, and ownership agreement exist.
- `BITOP-00` | status: blocked | execution: inactive | next: Remain lifecycle-blocked until STACK-BITOP-53 validates every mandatory lane; then a coordinator plan revision may close the parent as validated.
<!-- todo-orchestrator:v2-managed:end -->
