# Baseplane CUDA lab: make biological conditionality an executable data structure

**Recommendation:** start with four small kernel experiments, not a new encoder framework. The most revealing is **Rethread**: a warp changes from serving many candidate objects to collectively serving one selected object's floating representation. **BitLift**, **CarryFold**, and **Rendezvous** test three different ways of turning information structure into operations the hardware can perform coherently.

These names identify local experiments only. None is a proposed replacement name for Baseplane. The code is a prepared starting point, not performance evidence or a trained model. Exact preparation checks and remaining work are in `evidence/verification.json` and `planning/EPIC.md`.

## What the live source actually supports

Project Control observed Baseplane at commit `7846247`, dirty working tree, Todo revision 131. Its stable base is packed sequence, validity/coordinate contracts, exact matching, compact outputs and scalar references. `Baseplane::seq` uses C++17 and optionally CUDA, with Volta sm_70 as the native target. The existing warp-conversion routine is not the stable exact-count API. The historical count benchmark explicitly excluded packing, transfers and dense output; it cannot be used to advertise embedding throughput. [B1, B2]

The older BitOp ledger prohibits several things the new long-term goal will eventually need. The plan therefore explicitly proposes a **scoped experimental exception** for tiny parameterized floating transforms and value-dependent routing, without changing production ownership or authorizing a training runtime. The exception is inspectable in the plan diff. No governance, task state, source or GPU process has been changed remotely by preparing this package.

## The research-derived constraints that shaped the choice

NVIDIA's warp operations provide explicit collective communication and grouping, not an excuse to assume implicit lockstep. Correct participant masks and source lanes are part of the algorithms. `__match_any_sync` is usable on sm_70; the newer hardware warp-reduce intrinsics are not a V100 shortcut. Keep the 12.9 toolchain: CUDA 13 removed Volta offline compilation. [N1, N3, N6]

The PTX `lop3` instruction gives an instructive bridge from Boolean descriptions to word-level operations, but its truth table is an instruction immediate. We compile fixed Boolean primitives, not a pre-discovered biological vocabulary. Their masks and floating coefficients can vary with input or model parameters. [N2]

Register exchange, scan and warp-aggregated compaction are existing techniques. NVIDIA also notes that its compiler can aggregate atomics automatically. Therefore a handwritten primitive is not a novelty or speed claim; the right comparison is an equivalent compiled formulation, including all routing costs. [N4, N5, N7]

Learned segmentation and multilevel sequence abstraction already have substantial precedent, including H-Net's content-dependent hierarchy. Selective state-space work supplies another precedent for input-dependent recurrent computation, while Mixture-of-Depths and MegaBlocks address different forms of selective execution. Our opportunity is a **sequence-grounded, fine-grained execution experiment**, not a claim to have invented adaptive models. [P1–P4]

## The four experiments

### 1. BitLift — an exact Boolean basis feeding floating-point representations

Three bit planes describe 32 current items. Evaluate all eight Boolean combinations in parallel, count them and combine those counts with floating coefficients. One warp can map eight combinations across four output channels. The compiled primitive is tiny; what the bits signify is supplied at runtime, so the same operation can consume sequence-derived predicates now and predicates of lower-level embeddings later.

The creative part is the **change of representation**: cheap conditional evidence remains bit-parallel until the point where numerical mixing is useful. It is not a hard-coded motif inventory and not binary weights masquerading as FP embeddings. The prepared comparison is a warp/LOP3 path against one-thread-per-tile ordinary C++ bit logic.

Its danger is unusually clear: a histogram erases arrangement. The scalar tests deliberately show two different inputs with identical outputs. That makes it a candidate side channel or local primitive, never the sole claim to preserve the genome. The experiment must assess task-sensitive errors and exact-source revisits before promotion.

### 2. CarryFold — conditional history expressed as an associative object

Represent a small state update as `s' = a*s + b`, componentwise. A reset is simply `a = 0`. Two consecutive updates compose into another update of the same type. Shuffle scans can combine those objects; tile summaries themselves are scanned at the next level, then their carries are applied back. The same algebra therefore crosses hardware boundaries without declaring them biological boundaries.

The important distinction is between **learning an update from the current input** and serially branching through its execution. Once the coefficients are available, the restricted recurrence can be regrouped exactly over real arithmetic. This does not parallelize an arbitrary nonlinear recurrent program; any nonlinear layer not closed under the composition must remain outside it. Floating reassociation also needs an error envelope.

The prepared reference includes multiple levels and adversarial 31/32/33 and 1023/1024/1025 boundaries. Completing the prototype requires on-device coefficient/gate generation and a CUB baseline. Input-dependent affine scans are prior art; the research question here is whether a useful Baseplane summary can retain a composable execution form.

### 3. Rethread — change what the lanes mean when the information changes

First, lanes evaluate cheap properties of many records. A ballot and rank calculation build a bounded queue of records selected for additional work. Then a whole warp handles the channels of each selected floating representation. The queue carries record identity; results scatter back to the original order. Every record retains a cheap path, so declining refinement is not deleting the input.

That is more interesting than merely removing an `if`: **a change in representational resolution becomes a change in parallel work assignment**. The prepared richer operation is an ordinary small 4→32→32 floating transform, not artificial dummy arithmetic. It is compared with both a per-thread implementation and an uncompacted warp implementation; the second baseline prevents crediting compaction for gains that actually came from cooperative arithmetic.

The producer and consumer are separate same-stream launches with caller-owned buffers. There is no spinning global runtime, device-side allocation or child-kernel launch. Count resets, cheap-path writes, compaction and consumer launches all belong in the measured pipeline. The queue is unordered, but reconstruction is by original index; overflow is an explicit error, not a license to silently drop evidence.

### 4. Rendezvous — online peer sets as warp masks

Let each currently represented object produce a small routing key. `__match_any_sync` identifies same-key peers already resident in a warp. A peer mask can drive an exact group reduction and redistribute a candidate group summary while retaining each member's distinct state and origin. A comparison kernel examines all lane pairs with ordinary shuffles.

This is a deliberately small experiment in **forming computational neighborhoods from current information**. It has no chromosome-based branch. But equal keys are candidate grouping, not semantic equivalence, and the hardware does not find distant genome members for free: only the 32 presented candidates are visible. The follow-on experiment must include bounded online candidate movement and its traffic, collision and missed-partner costs.

A useful adversarial case supplies remote source IDs with related features, plus adjacent distractors. Physical packetization must not dictate the claimed biology. Offset and permutation tests reveal whether a result depends on a warp boundary rather than its information.

## Breadth considered, and why the first epic stays small

| Direction | Decision | Reason |
|---|---|---|
| Boolean-to-floating feature lifting | Selected: BitLift | Direct link from exact SIMD-like evidence to numerical representation; tiny setup and explicit loss witness. |
| Associative conditional histories | Selected: CarryFold | Adaptive state update without a branch tree; naturally reusable summaries. |
| Lane-role changes and compacted refinement | Selected: Rethread | Most direct test of hierarchical SIMT interpretation; counts real routing overhead. |
| Runtime peer-mask grouping | Selected: Rendezvous | Tests data-dependent neighborhoods, not fixed annotation groups. |
| Dynamic register-cache neighborhoods | Reserve | Interesting, but register indexing/spill complexity overlaps the selected gather/peer work. |
| Warp sorting or segmented regrouping | Baseline/alternative | Compare if peer-mask occupancy or collisions are unfavorable; not another mandatory project. |
| Bounded online cross-packet candidate exchange | Conditional extension of Rendezvous | Required before any meaningful nonlocal reach claim, but not a genome-wide discovery/index build. |
| Batch ragged refinements into WMMA tiles | Conditional extension of Rethread | Only after measured queue density and dimensions justify padding, packing and precision conversion. [N8, P4] |
| Adaptive precision with residual detail | Deferred | Information-budget question deserves separate accuracy work; too easy to win by changing the task. |
| Cheap speculative pass with later repair | Deferred | First establish the gate’s false-negative cost and revisit contract. |
| Persistent multilevel queues | Deferred | Unnecessary synchronization/termination machinery for the first conceptual tests. |
| CUDA Graph replay | Later execution optimization | Replay may reduce a stable launch envelope's overhead; not a new representation mechanism. |
| Newer-architecture TMA/cluster-dependent designs | Excluded from baseline | Would make the first result contingent on different hardware. |
| FPGA translation | Background only | The present request is CUDA; no hardware or toolchain workstream. |

Selection is editorial judgment, not a measured ranking. The most creative result may be a failed but informative formulation. Do not expand this table into fourteen task trees.

## What success would actually establish

First establish exact agreement with the defined scalar operator, including empty inputs, invalid masks, tails, overflow and reorder restoration. Next establish whether an instruction-level formulation beats a strong equivalent baseline at a real density/width, including preparation and movement. Finally establish whether a two-level use retains information needed by a held-out synthetic question, rather than merely producing fewer bytes.

The package's initial GPU driver measures device-resident complete pipelines, not allocation/H2D/D2H end-to-end cost. Those excluded phases must be reported separately before an application-level performance claim. A synthetic benchmark is not biological validation. No full-genome model, complete encoder, trained hierarchy or CUDA speedup is asserted by this package.

A good final outcome is **one or two compelling demonstrations**, with the other experiments retained as short evaluated-not-promoted notes. Nothing requires putting all four in the CV or making Baseplane support four permanent backends.

## Source key

`evidence/sources.json` contains full titles, URLs, inspected source paths, observation timestamps and claim boundaries. N = primary NVIDIA documentation; P = original research; B/T = live Project Control evidence. Descriptions of the four proposed experiments are this package's design synthesis, not results reported by those sources.

Readable reference links: [evidence/REFERENCES.md](evidence/REFERENCES.md).
