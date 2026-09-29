# Baseplane CUDA lab — completed experiment record

This lab tested four isolated ways to organize sequence-derived information
for computation. The experiments are complete; the authoritative outcome and
limits are in the [selection report](results/selection/report.md).

## Read the result

- [Selection report](results/selection/report.md): combined-source replay,
  measured comparisons, decisions, and scientific limits.
- [Final source hashes](results/selection/source_hashes.json): identifies the
  source used by the combined replay.
- `results/selection/final-bench-*.json`: final 1,025- and 65,536-item timing
  receipts.
- `results/selection/final-{memcheck,initcheck,racecheck,synccheck}.json`:
  final sanitizer receipts.
- Per-experiment reports and raw records under `results/` preserve each
  experiment's source identity. Those case receipts are not interchangeable
  repeats of the later combined-source replay.

The bounded finding is a restricted ordered affine summary (CarryFold) with
recoverable source intervals; CUB was faster than the custom scan in the
reported fixtures. BitLift, Rethread, and Rendezvous were not promoted. The
fixtures use untrained parameters and synthetic records. They do not validate a
biological model, general genome context mechanism, or learned genomic
geometry. No public Baseplane API changed.

## Reproduction and checks

The CPU reference gate does not require a GPU:

```sh
python3 experiments/cuda_lab/tools/run.py --phase host --case all --build-dir build-bitop-BP-CUDA-LAB-final-host
```

For a fresh CUDA replay, use the recorded CUDA 12.9 / `sm_70` configuration and
the repository's assigned GPU controller. Do not run GPU or benchmark phases
without its exclusive resource assignment. The existing combined replay can be
checked without launching it again:

```sh
python3 experiments/cuda_lab/tools/check.py --all-results
```

The completed standalone lab epic is separate from the older Baseplane BitOp
work queue. Its results do not complete or supersede those tasks. Future
scientific promotion would require sequence-grounded held-out evaluation,
uncertainty, online candidate construction, full movement and revisit costs,
and a reviewed interface.
