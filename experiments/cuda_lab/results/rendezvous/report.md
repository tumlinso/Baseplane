# Rendezvous result: evaluated, not promoted

Synthetic untrained fixture; CUDA 12.9.86 on V100 sm_70. Host reference tests, Project Control CUDA gate and all four Compute Sanitizer modes passed. The fixed 128-candidate packet was assembled by a test oracle with distant source IDs spanning 12,700,381 units. No runtime method for finding arbitrary distant candidates was evaluated.

The device-resident timed paths use the same 128 candidate values and keys. The CUB path includes sort, gather, warp match and scatter, with preallocated scratch (the CUB query reported 1 byte for this fixed size). The no-regroup path measures local match on the original packet; exact all-pairs checks the entire packet. Allocation, host packet construction, H2D/D2H and nonlocal candidate acquisition are excluded. No end-to-end routing claim follows.

| path | median ms | p95 ms |
|---|---:|---:|
| supertile_no_regroup | 0.009216 | 0.01024 |
| supertile_all_pairs | 0.011264 | 0.012288 |
| cub_sort_gather_match_scatter | 0.031744 | 0.031744 |
| higher_level_cub_regroup | 0.033792 | 0.033792 |

Across the fixed packet there were 994 directed same-key partner relations. Local matching missed 2; sorted regrouping missed 120 because a large equal-key group crossed a warp boundary after sorting. The exact all-pairs comparator retained all defined-key partners. The controlled 31/32 lane test changes placement while preserving source identities and confirms that a match can disappear solely at a warp boundary. A second pass reuses the same grouping machinery over first-level means with explicit source maps, but adds no evidence that its groups are biologically meaningful.

For the 65,536-record local-only test, match-any was 0.02048/0.019456/0.014336 ms for key modes 0/1/2; pairwise was 0.016384 ms in each. The match-any primitive wins only the all-unique key mode here. Neither local primitive provides arbitrary nonlocal search. Equal keys can collide for distinct values and source identities; the original representations remain available.

The CUB grouping path is slower than both alternatives on this fixture and worsens partner recall. Retain only the bounded tests as evidence, with no promotion. Candidate packet formation, cross-device movement, real learned keys, and biological quality remain unmeasured.

Source SHA-256: driver 035af723b47588592d7f25528f920705cd74c394cc2bc54fdb0619c5296e1f66; kernels 53b687404a99f8e81176c039bcf976d29b563355e60eb5dc086959e9448cadd1; runner 8f8cac4e76a24abab1b57d846ca044533de70ec17ebe3770f72e2ed758d07a5b.
