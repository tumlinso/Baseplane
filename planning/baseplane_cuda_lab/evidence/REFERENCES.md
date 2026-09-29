# Primary references

Retrieved 28 September 2026. Numbered keys used in RESEARCH.md.

**N1 — [CUDA C++ Programming Guide 12.9.1](https://docs.nvidia.com/cuda/archive/12.9.1/cuda-c-programming-guide/index.html)**

Warp vote/match/shuffle contracts; Volta independent thread scheduling; match_any supports sm_70+, reduce_sync needs sm_80+; WMMA collective constraints.

**N2 — [PTX ISA 8.8](https://docs.nvidia.com/cuda/archive/12.9.1/parallel-thread-execution/index.html)**

lop3.b32 implements a ternary Boolean lookup with an instruction-immediate truth table; shf is a funnel shift. Fixed primitives are not pre-discovered genomic biology.

**N3 — [Using CUDA Warp-Level Primitives](https://developer.nvidia.com/blog/using-cuda-warp-level-primitives/)**

Explicit participant masks, warp-local collectives and Volta-safe synchronization.

**N4 — [Register Cache: Caching for Warp-Centric CUDA Programs](https://developer.nvidia.com/blog/register-cache-warp-cuda/)**

Shuffle-based register exchange is prior art; register pressure and spills can negate benefits. This does not establish a free arbitrary dynamic register array.

**N5 — [Optimized Filtering with Warp-Aggregated Atomics](https://developer.nvidia.com/blog/cuda-pro-tip-optimized-filtering-warp-aggregated-atomics/)**

Ballot/rank and leader reservation are established; compilers can aggregate atomics already. Stable and unordered selection have different contracts.

**N6 — [CUDA Toolkit 13.0 release notes](https://docs.nvidia.com/cuda/archive/13.0.0/cuda-toolkit-release-notes/index.html)**

Offline compilation and library support for Volta removed in CUDA 13; use the recorded 12.9 toolchain, not latest by default.

**N7 — [CUB WarpScan](https://nvidia.github.io/cccl/cub/api/classcub_1_1WarpScan.html)**

Library scan baseline; use the CUB shipped with CUDA 12.9, not unpinned latest CCCL.

**N8 — [CUTLASS: Fast Linear Algebra in CUDA C++](https://developer.nvidia.com/blog/cutlass-linear-algebra-cuda/)**

Volta WMMA and hierarchical tiling exist; no justification to force tiny ragged operators onto Tensor Cores before measurement.

**P1 — [Dynamic Chunking for End-to-End Hierarchical Sequence Modeling](https://arxiv.org/html/2507.07955v2)**

Learned content/context-dependent hierarchical chunking, including DNA experiments, already exists. Do not claim dynamic hierarchy itself as novel.

**P2 — [Mamba: Linear-Time Sequence Modeling with Selective State Spaces](https://arxiv.org/html/2312.00752v2)**

Input-dependent state propagation with hardware-aware recurrent computation is established. Affine/selective scan is an algorithmic reference, not a Baseplane invention.

**P3 — [Mixture-of-Depths](https://arxiv.org/abs/2404.02258)**

Content-dependent allocation of computation under a bounded budget; selection quality and routing overhead matter.

**P4 — [MegaBlocks: Efficient Sparse Training with Mixture-of-Experts](https://arxiv.org/abs/2211.15841)**

Packing variable routed work without discarding tokens motivates a conditional tile-batching extension; no training framework is copied into Baseplane.

Live repository paths, fingerprints and observation details are retained in sources.json. The current user-stated scientific intent is not interchangeable with implemented-code evidence.
