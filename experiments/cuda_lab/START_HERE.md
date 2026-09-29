# Baseplane CUDA lab — ready-to-adopt experiment epic

Read **RESEARCH.md** for the ideas and **planning/EPIC.md** for execution. Four experiments and their scalar references/CUDA starting kernels are already written; a new agent does not need to reconstruct the conversation.

**Current package status:** host references tested; CUDA source/driver prepared but not compiled or executed in the preparation environment. No GPU timings, trained models or live repository changes are claimed. `evidence/verification.json` records checks actually run. `evidence/native-plan-preview.json` records the real read-only native validation result.

## Start with Rethread

It tests a direct version of the guiding principle: lanes inspect many objects cheaply, then a selected object recruits a whole warp for richer floating computation. The other experiments test Boolean-to-float lifting, composable conditional histories and runtime peer sets. They are experiments, not commitments to Baseplane's final architecture.

## Install without touching existing work

From the downloaded package:

```sh
python3 tools/install.py --repo /actual/path/to/Baseplane
python3 tools/install.py --repo /actual/path/to/Baseplane --apply
```

The first call previews. The second creates only `experiments/cuda_lab/` and refuses conflicts. It does not edit root CMake, Git, Todo, GPU state or any external repository. Use the actual local path; no host path has been guessed.

## Import the native plan in the existing Baseplane authority

From the actual Baseplane repository root, using the canonical Project Control operator environment:

```sh
project-control plan validate --project baseplane --file experiments/cuda_lab/planning/epic.v2.json
project-control plan apply --project baseplane --file experiments/cuda_lab/planning/epic.v2.json
```

Before apply, review `planning/scoped_policy_delta.json`: it narrowly updates **two existing invariant texts** to permit isolated floating/routing probes. It does not unfreeze the old ABI, enable a trainer or create a new production numerical owner. Recheck the current task/claim/Git context and the diff. Do not edit SQLite or generated Todo projections. Do not reset or migrate the existing Baseplane authority.

## First runnable gate

```sh
python3 experiments/cuda_lab/tools/run.py --phase host --case all   --build-dir build-bitop-BP-CUDA-LAB-01-host
```

For CUDA use the existing CUDA **12.9** toolchain and **sm_70**. CPU-only work needs CMake, C++17 and Python 3.10+; no pip packages are required. The `build-cuda`, `gpu` and `bench` phases are separate. Native CUDA gates supply an assigned visible GPU and build step. The runner refuses GPU execution without an externally assigned `CUDA_VISIBLE_DEVICES`; that variable is not itself a lease, so use the real scheduler.

Only run benchmarks and Compute Sanitizer under the existing exclusive resources. No GPU work was launched remotely to prepare this package.

## What to edit

All scope is inside this lab. `cuda/kernels.cuh` contains the small candidate kernels; `cuda/driver.cu` has equivalent comparisons and preallocated timing paths; `host/reference.cc` contains oracles; `tests/host_tests.cc` exercises edge cases and loss witnesses. Experiment briefs identify the small missing pieces—CUB baselines, live input/gate adapters, hierarchy and quality probes. Do not hide those gaps behind passing scalar tests.

A clean result may be “the compiler/library already does this better.” Keep that evidence and move on. Finish with one or two convincing demonstrations, not four new subsystems.

## Preparation validation

The exact native plan passed the live read-only validator at revision 131; it was not applied. CPU reference tests and 15 package tests passed. CUDA compilation and execution remain unperformed. See [evidence/VALIDATION.md](evidence/VALIDATION.md) for the precise boundary.
