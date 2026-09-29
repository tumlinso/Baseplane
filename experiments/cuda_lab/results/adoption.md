# BP-CUDA-LAB-01 adoption receipt

- The isolated lab was installed and the two scoped Todo invariant extensions were reviewed before import. Existing exact APIs and root CMake were not edited.
- The required host gate `BP-CUDA-LAB-01-HOST` passed through Project Control on 2026-09-29.
- The CUDA source built for `sm_70` with the installed CUDA 12.9.86 compiler by setting `CUDACXX=/opt/nvidia/hpc_sdk/Linux_x86_64/26.1/cuda/12.9/bin/nvcc` and running `tools/run.py --phase build-cuda --build-dir build-bitop-BP-CUDA-LAB-01-cuda129` from the repository root. The default `/usr/bin/nvcc` is CUDA 12.0 and failed compiler identification; future build gates need the explicit 12.9 compiler.
- This build is not CUDA correctness, sanitizer, benchmark, or scientific evidence.

Remaining bounded gaps: BitLift needs sequence/float input adapters and a two-level quality probe; CarryFold needs on-device gate production and a CUB scan baseline; Rethread needs a CUB selection baseline and coarser-level refinement; Rendezvous needs bounded candidate regrouping and a no-regroup baseline. All four still need device qualification, full cost accounting, and truthful result receipts before selection.
