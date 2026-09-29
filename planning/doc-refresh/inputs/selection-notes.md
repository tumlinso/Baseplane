# Baseplane evidence selection

## Displayed: CarryFold as one historical control result

- **Disposition:** one selected public result page; it is a bounded synthetic exactness/cost control, not a promoted production method or a positive speedup claim.
- **Source:** `experiments/cuda_lab/results/selection/report.md`, observed in source commit `e37f85b4f651a1cddfcb481610ecff94e735d03f`; current report SHA-256 is recorded in `inputs/results/bp-carryfold.json`.
- **Measurements:** use only `final-bench-1025.json` and `final-bench-65536.json`, the later combined-source replay. Each records 31 samples, CUDA-event medians/p95, scalar checks, and V100 / CUDA 12.9.86 metadata. Do not combine these with earlier per-case receipts.
- **Source identity:** all six hashes in `results/selection/source_hashes.json` were recomputed against current lab source and match. The raw report, both bench receipts, source hash record, and four final sanitizer receipts are tracked, small, reachable files; their SHA-256 values are bound into the result record. Raw receipts are unchanged. Their command field contains the original build path; the page supplies a portable relative reproduction recipe and clearly marks it historical.
- **Scope:** retain the CUB-faster comparison at both sizes, and the restricted two-level affine composition/source-interval correctness finding. Exclude allocation, host fixture construction, transfers, result interpretation, biology, trained parameters, and organism-scale claims.
- **No rerun:** documentation work did not run the benchmark or sanitizer campaign.

## Omitted as a separate page: Rethread

Keep its full density sweep and negative decision in `experiments/cuda_lab/results/selection/report.md` and the tracked final all-case receipts. It is not selected as a second highlighted study; no per-case Rethread raw source identity is merged into CarryFold.

BitLift and Rendezvous likewise remain linked from the original selection report rather than being expanded into more public pages. No samples, intervals, or uncertainty bands were synthesized.
