# The experiment charter

Baseplane aims at online, hierarchical, floating-point representations of genome-wide information, shaped jointly by biochemical organization and accelerator execution. The question is not how to run a fixed motif catalogue faster. It is whether conditional information flow can emerge cheaply while an unfamiliar sequence is being represented.

Cellerator may discover conditional system structure ahead of execution and compile kernels/assemblies, with eventual runtime state gating. Baseplane may compile an operator vocabulary and learn parameters across training examples, but may not assume an oracle has first discovered the new genome's useful biological hierarchy. Multiple bounded encoding passes, local lookahead and runtime regrouping are allowed if their cost is counted; “online” is not a commitment to strict single-pass causal generation. Any future autoregressive mode needs its own no-future-leakage contract.

Local genomic adjacency should be cheap to consider because of layout and execution, not an explicit distance-weighting law. It is not a rule that only nearby things matter, nor a claim that sequence distance equals three-dimensional molecular distance. There is no cis/trans API split. Arbitrary remote source identities remain legitimate, but moving their representations across memory or devices is not free.

The same adaptation should eventually act on higher-level floating representations, not only on individual bases. Warp size and block boundaries are scheduling choices, not biological segmentation labels. Shared state may condition gates; no particular current annotation vocabulary is the permitted universe of function.

## Keep the strongest intuition, reject the strongest overclaim

Use changing masks, operands, queues and group membership to keep execution coherent. This is not literal CPU branch prediction, nor a requirement for zero branch instructions. A uniform loop or coherent branch can be right. Predicating two expensive alternatives can execute both, while compaction can cost more than the work it saves. Inspect SASS and full pipeline cost rather than counting `if` in C++.

An exact source tape, a coordinate/child mapping and a compressed embedding are three distinct things. Retaining the source permits revisiting it; it does not prove the embedding retained every task-relevant fact. The first whole-input pass costs at least reading that input. The ambition concerns expensive representation and interaction costs, not magical sublinear ingestion or lossless fixed-size compression of arbitrary genomes.

## Boundaries for this epic

Create only an isolated `experiments/cuda_lab/` tree. Leave frozen exact APIs, root CMake, existing dirty files and other repositories alone. This is CUDA-first on the recorded V100/sm_70/CUDA12.9 configuration. No FPGA implementation, cloud dependency, new GPU, model download, full training framework, parser, genome database or distributed runtime is required. FPGA portability may be noted but is not a task.

The supplied weights are deterministic untrained probes. Their presence does not establish a trained embedding model. Tiny parameterized arithmetic tests are permitted here; production numerical ownership remains with Cellerator. GlassHelix can eventually supply supported mechanisms or state constraints, but these experiments must not wait for that feedback loop or equate a routing decision with a discovered mechanism.

A failed experiment is a valid result. Preserve a small negative result when it explains an instruction/representation trade-off. Do not turn all four prototypes into permanent architecture simply because they run.
