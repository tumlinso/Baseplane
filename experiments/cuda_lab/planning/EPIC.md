# BP-CUDA-LAB-00 — four small CUDA questions, not a new platform

The native file is `epic.v2.json`: one epic and **six executable outcome tasks**. The four experiments share a tiny reference/driver, but answer different questions.

| Task | Outcome | Dependency |
|---|---|---|
| BP-CUDA-LAB-01 | Adopt isolated lab; scalar/build contract check | None |
| BP-CUDA-LAB-10 | BitLift correctness, cost and information-loss result | 01 |
| BP-CUDA-LAB-20 | CarryFold on-device gates, scan comparison, hierarchy result | 01 |
| BP-CUDA-LAB-30 | Rethread equivalent mapping/queue comparison and two-level probe | 01 |
| BP-CUDA-LAB-40 | Rendezvous candidate-generation/peer result | 01 |
| BP-CUDA-LAB-90 | Select at most two ideas; one bounded demonstration or useful negative report | 10,20,30,40 |

Recommended serial order: **01 → 30 → 20 → 10 → 40 → 90**. Dependencies deliberately do not force an unproductive experiment to block all the others. The default is one agent; the shared prototype file is too small to justify a coordinator fleet or four concurrent editors.

Each prototype already has source and scalar references. Its task is to qualify and extend just enough to reveal the mechanism, not rediscover the specification. Use `evaluated_not_promoted` when it runs correctly but is not useful. A failing implementation is not a negative scientific result: repair the bounded correctness issue or record an explicit blocker.

The required native CUDA gate compiles with the existing scheduler and runs a 1025-item correctness case on one assigned GPU. The broader tail/boundary matrix and sanitizer results are outcome evidence, not dozens of new tasks. Profiling/benchmarking require the existing serialized resources; do not occupy all four GPUs to run a small kernel experiment.

The result checker verifies required receipt categories, not scientific truth by itself. Never manufacture measurements or mark unrun gates passed. A CUDA source file is not execution evidence. The supplied checks intentionally fail `--all-results` until actual result records exist.
