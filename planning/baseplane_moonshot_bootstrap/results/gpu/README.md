# Controller GPU correctness checks

[Qualified checks](qualified-checks.json) records four executable identities and their actual invocations: three Baseplane probes covering E01–E04, E18 and E45–E48, plus the hardened Cellerator tensor probe covering E21–E24. All returned zero and reported semantic comparison success. These are nine Baseplane card checks and four numerical provider card checks on tiny fixtures. No benchmark metric, speedup or biological validation is claimed.

| Probe | Source identity | Binary SHA-256 |
| --- | --- | --- |
| Baseplane logic | Family source `9b9551f58e5e8ce05c4acdec500ccdfc4bb90e8b` | `60d52cbe541dd0d413efbe7cd70405b39e517fa98912edfae61d6811e99d8984` |
| Baseplane rendezvous | Family source `ad5f74eb0aac6d7ef26ddb87c477df83eb5f6cba` | `812d44064a8b5503df0cd494d759b3acf7b4df8acd7dbb77775403b130995c22` |
| Baseplane machine | Family source `6c2cf561d454215453c85b19444422404f15447d` | `07d0e12db50d01439f397c8c14146830f41fe3f9f3cc98f2219e68e978563694` |
| Cellerator tensor, hardened | Comparator source `f6425a78ad728212ba3d24a40cfa1630d2ad18b8` | `599954303efa6a859b9168c7767ef7516858d74f954f9a9b1c56fa79ff84801d` |

The Baseplane build caches identify the original family workspaces; their family code remains identical to the listed commits. Source hashes are recorded in qualified-checks.json. The controller snapshot at invocation was Baseplane `fcd99cba67d501bd443238bab55d250470cbdb09`. This preserves the distinction between the controller checkout and separately compiled family binaries. A new full dependency fingerprint is not reconstructed from historical builds.

## Historical run and supersession

[Initial batch results](initial-batch-results.json), [initial controller receipt](initial-controller.json) and the [actual historical script](historical-batch-script.py.txt) preserve the original `python3 /tmp/moonshot_gpu_batch.py` invocation with its four build-specific absolute paths. Native evidence ID: `a5e31189-5005-48dd-a9dc-ef3e4cebdd6a`.

The initial fourth executable was Cellerator tensor SHA-256 `138e79cf79eee487108dadecbcadb94578f0ab873d07e10038e210f9c74abf6c`. Its comparator did not reject NaN. That tensor result is superseded by the hardened binary above, whose finite actual/expected checks and host-only comparator self-check preceded the final GPU rerun. [Final controller receipt](final-tensor-controller.json) and [final tensor receipt](final-tensor-receipt.json) preserve that successful rerun, evidence `fba3e31c-a618-4903-9dfb-f2559c85e11e`. The latter receipt is copied unchanged from Cellerator commit `b2a65353d543a8d77d051be2d277d0c6ba4371dc`.

Original family receipts remain historical authored/compiled records. This directory adds controller runtime evidence without rewriting those receipts or claiming that every authored CUDA mechanism ran.

## Assigned resources and quiescence

[Initial resources](initial-resources.json) and [final tensor resources](final-tensor-resources.json) preserve native controller lease observations, exact resource samples and interlock acquisition/release output. Both recorded three required consecutive idle observations, `CUDA-QUIESCENCE/1` state `quiescent`, `uncontaminated: true`, no foreign processes and controller contamination flag zero on assigned accelerator `GPU-21131915-1488-23af-38dd-1743ae1f5cc8` (Tesla V100-SXM2-16GB, compute capability 7.0).

Both pre-run samples were idle. Post-run samples briefly reported activity and then zero utilization, with no foreign process observed. This is a record of those intervals, not a statement about current quiescence. Native controller foreground leases and the host-global CUDA interlock held during execution. Quiescence bookkeeping elapsed fields describe resource acquisition; they are not benchmark results.

## Portable reproduction

[run_gpu_checks.py](../../../../experiments/moonshot/python/run_gpu_checks.py) accepts explicit aggregate Baseplane and Cellerator build directories and resolves the four known target paths. It has no historical temporary-path discovery. It plans commands by default and launches only with `--run`. The controller must reserve/preempt resources and configure visibility before invoking it; the runner does not acquire or change leases/interlocks. The CE harness selects visible device zero, so this runner uses visible device zero consistently for all four probes.

```sh
python3 experiments/moonshot/python/run_gpu_checks.py \
  --bp-build-dir /explicit/path/to/bp-build \
  --ce-build-dir /explicit/path/to/ce-build
```

For actual execution, use that command with `--run --output /explicit/path/to/new-receipt.json` as the foreground controller's command after assigning resources. Existing output files are preserved. Build the targets with their opt-in CUDA flags and an sm_70-capable compiler first. The runner records actual binary hashes, commands and outputs, stops on failure and has a per-probe timeout. Planning/argument checks were exercised without launching a GPU; historical runtime evidence came from the earlier controller scripts, not this new runner.
