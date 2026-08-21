

<!-- todo-orchestrator:v2-managed:start -->
# Todo Status v2 Projection

Project revision: `128`

## Workstreams
- `STACK-BITOP-01` | status: done | execution: closed | next: Verify the recorded evidence against current Git/source without changing implementation; run context, changes, guard, and audit; then complete validated.
- `STACK-BITOP-02` | status: done | execution: closed | next: Remain blocked until CE-ARCH-10 freezes the shared biological ABI; then author only the Baseplane-local ADR and explicitly import rather than redefine the external identity/operand contract.
- `BP-BITOP-03` | status: done | execution: closed | next: Use build-bitop-BP-BITOP-03-cpu and -cuda, run actual current targets, acquire one GPU and exclusive benchmark resource only for the benchmark gate, and record exact toolchain/commands/checksums in docs/bitop_baseline_evidence.md.
- `STACK-BITOP-04` | status: done | execution: closed | next: Run the required Baseplane audit gate, verify STACK-BITOP-02 and BP-BITOP-03 evidence, reach BITOP_BASELINE_AND_ARCHITECTURE_READY, and complete validated without claiming implementation work.
- `BP-BITOP-10` | status: done | execution: closed | next: Implement only the three owned files; resolve BITOP-CHUNK-COORDINATES; compile/run the standalone test in build-bitop-BP-BITOP-10; hand off provisional declarations to BP-BITOP-13.
- `BP-BITOP-11` | status: done | execution: closed | next: Implement direct membership and reverse-complement oracle in owned files, with no IUPAC parser; compile/run standalone tests; hand off provisional ABI to BP-BITOP-13.
- `BP-BITOP-12` | status: done | execution: closed | next: Choose composition/alias versus new record without widening, add descriptors and static assertions/tests, reach BASEPLANE_SEQUENCE_EVENT_V1_FROZEN, and hand off the hash.
- `BP-BITOP-13` | status: done | execution: closed | next: Guard shared seams, reconcile three provisional interfaces, define/verify portable ops and limits, wire baseplaneBitOpContractTest, freeze baseplane-sequence-predicate-v1, and hand off exact hash/limits.
- `BP-BITOP-20` | status: blocked | execution: blocked_dependency | next: Implement oracle only in owned files; property-test against unpacked logic; hand off deterministic fixture interface to BP-BITOP-24.
- `BP-BITOP-21` | status: blocked | execution: blocked_dependency | next: Implement constant folding, mask/shift normalization, deterministic CSE/dead output elimination, exact/allowed specialization, family candidates, halo/density/scratch; do not bake unmeasured thresholds.
- `BP-BITOP-22` | status: blocked | execution: blocked_dependency | next: Implement correct repeated-shift baseline and ownership mask, test exhaustive boundaries/radii, hand off halo convention to CUDA lane.
- `BP-BITOP-23` | status: blocked | execution: blocked_dependency | next: Define stable canonical position/predicate order and unordered multiset mode, exact zero-capacity/overflow, and caller scratch contract in owned files.
- `BP-BITOP-25` | status: blocked | execution: blocked_dependency | next: Implement only generic segment vocabulary and CPU reference; reach BITOP_EVENT_AND_SEGMENT_ABI_READY and freeze baseplane-segment-v1.
- `BP-BITOP-24` | status: blocked | execution: blocked_dependency | next: Guard CMake, build deterministic generators/fixture schema, wire baseplaneBitOpCpuTest, run CPU-only acceptance, reach BITOP_CPU_REFERENCE_READY.
- `BP-BITOP-30` | status: blocked | execution: blocked_dependency | next: Implement candidates A-D in owned files, correctness first, run assigned-GPU tests and leased benchmark gates; preserve M=1 regression rule.
- `BP-BITOP-31` | status: blocked | execution: blocked_dependency | next: Implement candidates with explicit buffers/scratch; test CPU parity/invalid/strand; collect one-shot versus reuse crossover inputs without silently caching planes.
- `BP-BITOP-32` | status: blocked | execution: blocked_dependency | next: Implement global streaming baseline and conditional shared supertile, test cross-word/block/OOB/duplicate ownership, profile only under leased profiler.
- `BP-BITOP-33` | status: blocked | execution: blocked_dependency | next: Implement both explicit policies, CPU multiset parity, stable byte determinism, sparse-to-dense density cases, then run memcheck/racecheck with exclusive sanitizer resource.
- `BP-BITOP-34` | status: blocked | execution: blocked_dependency | next: Agree minimum adapter operations from frozen segment ABI, implement CPU parity/uneven/zero segments/explicit scratch and sm_70-safe kernels.
- `BP-BITOP-35` | status: blocked | execution: blocked_dependency | next: Guard shared seams, integrate CUDA lanes, wire baseplaneBitOpCudaTest, expose required_scratch/prepare/bind/run and inspectable backend choice, run CPU/CUDA/sanitizer fan-in, reach both executable checkpoints.
- `CE-BITOP-40` | status: superseded | execution: closed | next: No implementation. This task is terminally superseded by CellStack CE-ARCH-40 and CELLERATOR_BASEPLANE_COMMON_ABI_V1_READY.
- `CE-BITOP-41` | status: blocked | execution: blocked_dependency | next: Remain parked. A later Cellerator-owned plan revision may supersede or reactivate the concept only after all external foundations and benchmark evidence validate.
- `CE-BITOP-42` | status: blocked | execution: blocked_dependency | next: Remain parked pending external foundations and a new Cellerator-owned activation decision.
- `BP-BITOP-50` | status: blocked | execution: blocked_dependency | next: Guard shared benchmark/CMake seams, wire baseplaneBitOpBench, run serialized matrix/checksum/sanitizer/nsys evidence, set measured policy decisions, publish selector inputs and exact commands.
- `STACK-BITOP-51` | status: blocked | execution: blocked_dependency | next: Remain parked. Do not acquire gpu-all4 or benchmark resources until Cellerator publishes an explicit partition contract and a later plan revision activates validation.
- `STACK-BITOP-52` | status: blocked | execution: blocked_dependency | next: Guard all shared Baseplane and external Cellerator seams, reconcile generated docs/targets, validate install/CPU/CUDA/consumer targets; do not create Baseplane::bitop for branding.
- `STACK-BITOP-53` | status: blocked | execution: blocked_dependency | next: Acquire all integration locks, audit Baseplane and external repos, rerun focused CPU/CUDA/sanitizer/integration evidence, write docs/BITOP_FINAL_AUDIT.md, leave commit/push/pointer publication for separately authorized future work.
- `BP-BITOP-36` | status: blocked | execution: blocked_dependency | next: Remain parked until a named pattern proves halos/sparse continuation inefficient, state count bounded, and benchmark supplied.
- `CE-BITOP-43` | status: blocked | execution: blocked_dependency | next: Remain parked until prediction target, dataset, baseline, seed plan, and resource budget are recorded.
- `CE-BITOP-44` | status: blocked | execution: blocked_dependency | next: Remain blocked until native hashes stable and activation set; then work only under Cellerator/components/CelleraTorch.
- `CE-BITOP-45` | status: blocked | execution: blocked_dependency | next: Remain parked until concrete biological objective, edge source, baseline and sparse complexity budget exist.
- `CS-BITOP-60` | status: blocked | execution: blocked_dependency | next: Remain parked until stable ABI, demonstrated reuse/deployment need, regeneration-cost evidence, and ownership agreement exist.
- `BITOP-00` | status: blocked | execution: inactive | next: Remain lifecycle-blocked until STACK-BITOP-53 validates every mandatory lane; then a coordinator plan revision may close the parent as validated.
<!-- todo-orchestrator:v2-managed:end -->
