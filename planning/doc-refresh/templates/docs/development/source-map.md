# Source map: exact input, then experimental representation

| Question | Start here | What to look for |
|---|---|---|
| What is the current exact-sequence entry? | [CPU-safe include](../../{{P_API}}) | Public library entry; not the eventual full encoder API. |
| How are invalid bases, tails and coordinates represented? | [validity contract](../../{{P_VALIDITY}}) | Exact source semantics and chunk identity. |
| Where are reference and GPU paths? | [scalar](../../{{P_SEQ_CPU}}), [CUDA](../../{{P_SEQ_CUDA}}) | Same-operation comparisons and explicit buffer/stream behavior. |
| What tests protect exact behavior? | [sequence tests](../../{{P_TESTS}}) | Boundary, validity, capacity and output contracts. |
| Where are the online representation experiments? | [CUDA lab](../../{{P_LAB}}), [driver](../../{{P_LAB_DRIVER}}) | Four experiments, their current implementations and comparators. |
| What survived the experiments? | [selection report](../../{{P_LAB_REPORT}}) | One bounded representation candidate; measured failures are preserved. |

The [root build](../../{{P_CORE_CMAKE}}) and isolated lab build serve different purposes. Do not merge the lab into the exact library just to make the tree look uniform. A shared test/driver split is justified only if the actual files have become difficult to navigate; the default source disposition is to keep this useful separation.
