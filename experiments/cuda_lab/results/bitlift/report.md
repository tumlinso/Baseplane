# BitLift result: evaluated, not promoted

Synthetic, untrained fixture; CUDA 12.9 on V100 sm_70. Host tests, Project Control CUDA correctness gate, and all four Compute Sanitizer modes passed. The binary contains LOP3 and POPC instructions; see `sass.log`. The gate exercises the packed/valid adapter, lower-float threshold adapter, coarse second pass, validity and source mappings.

One item is a 32-position tile with three predicate planes. Both timed paths include packed-to-plane preparation, weight application, and device-resident execution. Allocation, initial host construction, H2D/D2H, and any source revisit are excluded. No complete end-to-end time was measured. The same prepared planes, weights, output type and normalization are used by both paths.

| tiles | path | median ms | p95 ms |
|---:|---|---:|---:|
| 1025 | packed_prepare_plus_warp_lop3_popc_fp | 0.008192 | 0.009216 |
| 1025 | packed_prepare_plus_thread_bitset_fp | 0.008192 | 0.009216 |
| 65536 | packed_prepare_plus_warp_lop3_popc_fp | 0.038912 | 0.039936 |
| 65536 | packed_prepare_plus_thread_bitset_fp | 0.016384 | 0.017408 |

At 65536 tiles, the thread path was 2.38 times faster (0.016384 versus 0.038912 ms). At 1025 tiles they tied. The 65536-tile packed input occupies roughly 0.75 MiB (three 32-bit planes plus validity per tile); prepared planes are another 1 MiB, and 32-float output is 8 MiB. Both paths share these buffers; CUDA build output and SASS are retained for register/instruction inspection. Fine-grained preparation and kernel timings were not separated, so these data support only the complete resident path comparison.

The held-out order-sensitive probe has indistinguishable rotated histogram embeddings despite different first-pattern labels. Thresholded lower-level children with the same signs and means 0.1 versus 0.9 lose magnitude difference 0.8. Exact source/child maps permit revisit but do not retain that information in the embedding; revisit cost was not measured. These are synthetic information-loss witnesses, not biological accuracy estimates.

The warp mapping has no measured cost advantage and the threshold/histogram summary loses order and magnitude. Keep the scalar and GPU implementation as an evaluated lab example; do not promote it to Baseplane production semantics or claim biological utility. No training, alternate hardware, or complete transfer timing was tested.

Source SHA-256: driver 989baa0da47bf172f69aae7ba5bdcd08ec5fe2408d9ff719f14354420e332c51; kernels f2abcd91abe080671c882adc5d73c4e68bea25592c4c6e014e2c942fdefa8439; runner 8f8cac4e76a24abab1b57d846ca044533de70ec17ebe3770f72e2ed758d07a5b.
