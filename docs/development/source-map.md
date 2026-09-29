# Source map: exact input, then experimental representation

| Question | Start here | What to look for |
|---|---|---|
| What is the current exact-sequence entry? | [CPU-safe include](../../include/Baseplane/dna2.hh) | Public library entry; not the eventual full encoder API. |
| How are invalid bases, tails and coordinates represented? | [validity contract](../../include/Baseplane/seq/dna2_validity.hh) | Exact source semantics and chunk identity. |
| Where are reference and GPU paths? | [scalar](../../src/seq/dna2_scalar.cpp), [CUDA](../../src/seq/dna2.cu) | Same-operation comparisons and explicit buffer/stream behavior. |
| What tests protect exact behavior? | [sequence tests](../../tests/seq) | Boundary, validity, capacity and output contracts. |
| Where are the online representation experiments? | [CUDA lab](../../experiments/cuda_lab), [driver](../../experiments/cuda_lab/cuda/driver.cu) | Four experiments, their current implementations and comparators. |
| What survived the experiments? | [selection report](../../experiments/cuda_lab/results/selection/report.md) | One bounded representation candidate; measured failures are preserved. |

The [root build](../../CMakeLists.txt) and isolated lab build serve different purposes. Do not merge the lab into the exact library just to make the tree look uniform. A shared test/driver split is justified only if the actual files have become difficult to navigate; the default source disposition is to keep this useful separation.
