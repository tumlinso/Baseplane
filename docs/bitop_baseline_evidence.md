# BP-BITOP-03 Exact-Count Baseline Evidence

Date: 2026-08-21 UTC

Repository commit: `768065b4bdd6693429a22a49c0fdeb2f1946180f`

This record preserves the current scalar and CUDA exact-count semantics and one
serialized, single-V100 observation. It does not optimize or revise the stable
`scan_exact_count_cpu` / `scan_exact_count_cuda` API.

## Toolchain and Hardware

- Host: Linux 6.8.0-138-generic, x86_64.
- CMake: 3.28.3.
- CPU compiler: GNU C++ 13.3.0 (`/usr/bin/c++`).
- CUDA compiler: NVIDIA CUDA 12.9.86
  (`/opt/nvidia/hpc_sdk/Linux_x86_64/26.1/cuda/12.9/bin/nvcc`).
- CUDA host compiler: GNU C++ 12.4.0 (`/usr/bin/g++-12`).
- CUDA architecture: `sm_70`; build type: `Release`.
- GPU lease: `gpu:0`, Tesla V100-SXM2-16GB, UUID
  `GPU-21131915-1488-23af-38dd-1743ae1f5cc8`.
- NVIDIA driver: 580.173.02.

## Commands and Results

The todo-orchestrator ran these commands in task-specific build directories.
All six configuration, build, and correctness gates exited 0:

```bash
cmake -S . -B build-bitop-BP-BITOP-03-cpu -DBASEPLANE_ENABLE_CUDA=OFF
cmake --build build-bitop-BP-BITOP-03-cpu --target baseplaneDna2Test -j 4
./build-bitop-BP-BITOP-03-cpu/baseplaneDna2Test

cmake -S . -B build-bitop-BP-BITOP-03-cuda \
  -DBASEPLANE_ENABLE_CUDA=ON -DCMAKE_CUDA_ARCHITECTURES=70
cmake --build build-bitop-BP-BITOP-03-cuda \
  --target baseplaneDna2CudaTest baseplaneDna2Bench -j 4
./build-bitop-BP-BITOP-03-cuda/baseplaneDna2CudaTest
```

Both correctness executables are silent on success. The CUDA compiler emitted
only its deprecation warning for offline compilation below `sm_75`; the
requested `sm_70` build and test completed successfully.

The benchmark command was:

```bash
./build-bitop-BP-BITOP-03-cuda/baseplaneDna2Bench \
  67108864 16 1 30 packed_word64_shifted_count 20260819 single_gpu
```

The run held the todo-orchestrator `v100-benchmark:exclusive` resource and
`baseplane-benchmark-driver` lock. The executable also acquired its repository
mutex at `/tmp/cuda_v100_benchmark.device0.lock`. The leases and mutex were
released when the command finished.

Observed output:

```text
sequence_length=67108864
motif_length=16
max_mismatches=1
iterations=30
seed=20260819
representation=packed_word64_shifted_count
cuda_devices=1
hits=1
elapsed_kernel_ms=0.325
bases_per_sec=206564397835.582
windows_per_sec=206564351664.841
```

There were 67,108,849 candidate windows, so the observed hit density was
approximately `1.49011645e-8`. The shifted-count kernel used 30 registers per
thread, no local memory, no static shared memory, and supported 1,024 threads
per block. The launch uses 1,024 bytes of dynamic reduction storage for its 256
threads.

## Transfer and Output Contract

The benchmark packs the sequence on the host, allocates device buffers, and
uploads 16,777,216 bytes once before warmup. Those operations are outside the
timed interval. Each `scan_exact_count_cuda` call asynchronously clears the
caller-owned 8-byte device counter and launches the shifted packed-word scan.
The reported 0.325 ms is the average CUDA-event interval for those operations
over 30 calls.

After timing, the benchmark copies one 8-byte hit count to the host. It does not
materialize a dense per-window output. Device allocation, host packing, H2D,
D2H, and end-to-end time are not included in this exact-count timing and must
not be inferred from it.

## Regression Rule

Correctness is a hard prerequisite: both the CPU reference gate and the CUDA
comparison gate must pass before a timing result is considered. Performance
comparisons must use the same command contract, source revision or recorded
source checksums, `Release`/`sm_70` build, one V100, and the serialized benchmark
resources. The plan contract permits no more than a 5% same-build median
M=1 exact-count regression unless a correctness or composability benefit is
documented. A comparison must report both median absolute time and throughput;
changes in GPU, driver, compiler, command, hit density, or source checksum
invalidate a direct comparison. The 0.325 ms observation above is provenance
for this run; it is not a cross-toolchain or cross-hardware threshold.

## Stable and Proof Paths

The stable exact-count surface is `scan_exact_count_cpu` and
`scan_exact_count_cuda`; the benchmark calls the CUDA wrapper, which owns no
allocation and dispatches `scan_motif_word64_shifted_count`. The raw
`scan_motif_warp32_unpacked`, `scan_motif_word64_reference`, aligned packed,
and inline-plane kernels remain proof/reference paths exercised directly by
tests or comparative benchmark modes. They are not the downstream exact-count
API and their dense outputs must not be used for this throughput claim.

`scan_exact_emit_cpu` and `scan_exact_emit_cuda` are stable compact-output
wrappers, but emit was not timed in this gate. Emit uses caller-owned storage
and explicit capacity, records-written, and records-dropped counters; the
count-only observation above uses only the caller-owned 8-byte counter.

## Checksums

```text
e49fff54e4d418458c412ae65d21a92f2ef863c211466af6cd46c79b61bdf3d5  CMakeLists.txt
11bdf92691f13e060d19a6f31f54c30c01545c96ca671ad184dfc64278f77156  src/seq/dna2.cu
3a01ab8e5650d0ba0ae16fdf7f5e2f48d0c1c420295de2ecfdda54a6ea0e8b7c  src/seq/dna2_scalar.cpp
a022577364e6efb19c4cf7bd09c88e4209189997bcad664fd6fb28203a427d85  bench/seq/bench_dna2.cu
46ba56ae6e1951da9efce122268a1a25660cd21bb14cf4dda70b05d4a87a2712  tests/seq/test_dna2.cpp
c49b8adc9f94579c67e9ac7533e1fae07bc7cc3c9b5b54b91b46a7bebb60e5e4  tests/seq/test_dna2_cuda.cu
d6f77ba80bca33ffc5d2194228801ff31a7b109a30270306d1575b259cecd43e  build-bitop-BP-BITOP-03-cpu/baseplaneDna2Test
42f9de6c9483fea4b1a511c221db7dbc126c93989fbfc14d7bd81e61b57d4615  build-bitop-BP-BITOP-03-cuda/baseplaneDna2CudaTest
6007081e3ad348f41f690ed1e1abd48a2249ad71b8ca3a968dff38f54b5e5034  build-bitop-BP-BITOP-03-cuda/baseplaneDna2Bench
```

## Orchestration Gate Contract

The original gate incorrectly treated the line-oriented `key=value` output as
JSON. The plan was transactionally validated and applied at revisions 43 and
44, changing only `BP-BITOP-03-BENCH` from structured benchmark evaluation to
exit-code command validation while preserving its command, inputs, resources,
lock, timeout, and Git-head tracking. The repaired gate reran under the
exclusive resources, exited 0, and passed with immutable evidence
`31b9e64c-624e-4683-8680-7bdf46f731c0`.
