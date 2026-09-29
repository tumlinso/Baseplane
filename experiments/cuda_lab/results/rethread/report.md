# Rethread result: evaluated, not promoted

Deterministic untrained fixture; CUDA 12.9.86, V100 sm_70, Release without fast math. The final Project Control GPU gate passed at 1025 records; all four CUDA 12.9 Compute Sanitizer modes passed. The gate checks equivalent CUB, direct warp and thread paths, overflow, and two-level reverse source routing.

One record is a 4-float representation; output is 32 floats. The timer covers device-resident variant launches, including cheap writes, queue work and refinement as applicable. It excludes allocation, initial host construction, H2D/D2H, scalar oracle and interpretation. At 65536 records, input is 1 MiB, output 8 MiB, and queue 0.25 MiB before scratch. Variants ran in fixed order. No end-to-end or fine-grained speed claim follows.

| records | target selected % | actual selected | thread ms | direct warp ms | compact warp ms | CUB select + warp ms |
|---:|---:|---:|---:|---:|---:|---:|
| 1025 | 0 | 0 | 0.011264 | 0.014336 | 0.019456 | 0.019456 |
| 1025 | 1 | 10 | 0.033792 | 0.01536 | 0.023552 | 0.023552 |
| 1025 | 5 | 51 | 0.033792 | 0.016384 | 0.022528 | 0.023552 |
| 1025 | 25 | 256 | 0.033792 | 0.02048 | 0.023552 | 0.023552 |
| 1025 | 50 | 513 | 0.032768 | 0.023552 | 0.026624 | 0.026624 |
| 1025 | 100 | 1025 | 0.033792 | 0.027648 | 0.033792 | 0.033792 |
| 65536 | 0 | 0 | 0.1024 | 0.14848 | 0.111616 | 0.113664 |
| 65536 | 1 | 655 | 0.11776 | 0.1536 | 0.123904 | 0.124928 |
| 65536 | 5 | 3276 | 0.130048 | 0.16896 | 0.151552 | 0.154624 |
| 65536 | 25 | 16384 | 0.123904 | 0.293888 | 0.284672 | 0.284672 |
| 65536 | 50 | 32766 | 0.124928 | 0.459776 | 0.45056 | 0.45056 |
| 65536 | 100 | 65536 | 0.285696 | 0.761856 | 0.786432 | 0.78848 |

The compacted path did not win in this fixture. Direct warp was faster at 1025 records for nonzero densities; thread was faster at 65536 records across the tested densities. The CUB stable selector agreed but did not improve pipeline time. Other input shapes or hardware remain untested.

A held-out scalar seed (20260929) selected 514 of 1025 records at threshold 0. Among 511 omitted records, 511 had L1 > 1e-3 versus always running the richer transform; total omitted L1 was 757.68. This is a synthetic information-loss witness, not biological validation. The GPU two-level check preserved source IDs through coarse selection but did not establish that coarsening retained task-relevant information.

Source SHA-256: driver e07f20ad279666f8654cbeaa0411824baafcd4ec8497e40abb9e1c0865e926e0; kernels d7d6c9c817fb9ac24d35832f8efaf56d529897b6cd01a4c407c863af2edb6d5a; runner 8f8cac4e76a24abab1b57d846ca044533de70ec17ebe3770f72e2ed758d07a5b.
