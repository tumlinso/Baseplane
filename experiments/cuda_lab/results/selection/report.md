# CUDA lab selection: one bounded candidate

All four isolated experiments were completed, with host and CUDA correctness, their required Project Control gates, four CUDA 12.9 Compute Sanitizer modes, same-operation comparators, and quality probes. A final integrated `sm_70` build was replayed across all four cases at 1,025 and 65,536 items, then all four sanitizer modes at 1,025. The final run passed; `source_hashes.json` identifies the combined source. Earlier case receipts preserve their own source hashes and are historical measurements, while the final replay checks the combined binary.

| Mechanism | Same-operation measured result | Quality/scope result | Decision |
|---|---|---|---|
| Rethread | At 65,536 records, the thread path beat compacted warp at every tested selection density (0, 1, 5, 25, 50, 100%). | Skipping richer work omitted nonzero synthetic outputs; source maps survived two levels. | Do not promote. |
| CarryFold (`scan`) | At 65,536 records, CUB producer+inclusive scan 0.059392 ms versus custom 0.070656 ms in the final replay. | Ordered affine composition and source intervals survive two levels; a same-mean order swap changes the result. Restricted affine family only. | Retain as one bounded candidate, using CUB for execution. |
| BitLift | At 65,536 tiles, packed preparation+thread 0.016384 ms versus warp 0.038912 ms in the final replay. | Rotated arrangements collide in histogram space; thresholding loses child magnitude. | Do not promote. |
| Rendezvous | Fixed 128-candidate packet: no regroup 0.009216 ms, exact all-pairs 0.011264 ms, CUB sort/gather/match/scatter 0.031744 ms at 1,025-item run. | Regrouping missed 120/994 directed same-key partners because groups crossed warps; packet was oracle assembled. | Do not promote. |

Raw times across rows are not comparable as throughput rankings: their units, outputs, candidate sets and semantics differ. Timings cover the stated device-resident pipelines with preallocated buffers. They exclude allocation, H2D/D2H, and end-to-end biological use; Rendezvous additionally excludes actual nonlocal packet discovery. All fixture weights and keys are untrained.

## Bounded two-level demonstration

CarryFold produces an affine coefficient/gate from current floating records on the GPU, scans with the left-then-right composition operator, and then scans four-record coarse summaries. The first-level result's source begin/end intervals are checked against a scalar reference. The final combined CUDA run re-executed this path at both sizes. It demonstrates exact composition for the defined affine summary and recoverable source intervals. It does not establish a general organism-scale context mechanism, learned genomic geometry, sequence causality, or a trained model. No public Baseplane API changed.

## Reproduction

From the repository root, build and run the host oracle:

```sh
python3 experiments/cuda_lab/tools/run.py --phase host --case all --build-dir build-bitop-BP-CUDA-LAB-final-host
```

Build the CUDA lab with the installed CUDA 12.9 compiler (ensure its `nvcc` is in `PATH`), then use the canonical CUDA controller for a leased GPU run. `results/selection/replay.py` records the six scheduled checks and their output paths:

```sh
python3 experiments/cuda_lab/tools/run.py --phase build-cuda --build-dir build-bitop-BP-CUDA-LAB-final-cuda
python3 experiments/cuda_lab/results/selection/replay.py parent
python3 experiments/cuda_lab/tools/check.py --all-results
```

The controller requires a quiescent assigned GPU. Do not invoke the `gpu` or `bench` phase without that lease. Final receipts are `final-{memcheck,initcheck,racecheck,synccheck}.json` and `final-bench-{1025,65536}.json` here. The CUDA controller receipt is `c517f7c0-9ad4-4740-8c53-55ca3ac9164d`.

The remaining work for scientific promotion is a genuine sequence-grounded task with held-out labels and uncertainty, a runtime method for constructing candidate contexts, full H2D/D2H and source-revisit costs, more hardware/input shapes, and an authorized interface review. These are outside this epic's bounded lab acceptance.
