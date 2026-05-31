# SequenceBits CUDA Profiling

Date: 2026-05-04

Hardware: 4x Tesla V100-SXM2-16GB, `sm_70`

Profiler: Nsight Systems 2025.3

Artifacts:

- `build/seq_dna2_nsys_old.nsys-rep`
- `build/seq_dna2_nsys_old.sqlite`
- `build/seq_dna2_nsys_shifted.nsys-rep`
- `build/seq_dna2_nsys_shifted.sqlite`

## Summary

The first packed CUDA scanner was weak because it was still a correctness
reference path, not a GPU streaming path. Each CUDA thread rebuilt a motif
window base-by-base, used the public mismatch-mask helper that collapses fields
through a loop, wrote one dense byte per scanned window, and the benchmark
copied that dense hit buffer back to host.

The fixed path keeps sequence in packed storage, forms each candidate window
with a 64-bit shifted load, compares packed fields directly with XOR / OR /
POPCOUNT, and reduces the hit count on device. The benchmark copies back one
8-byte count instead of a full hit array.

## Commands

Old reference packed scanner:

```bash
./build/baseplaneDna2Bench 67108864 16 1 30 packed_word64 20260504 single_gpu
nsys profile --stats=true --force-overwrite=true -t cuda \
  -o build/seq_dna2_nsys_old \
  ./build/baseplaneDna2Bench 67108864 16 1 5 packed_word64 20260504 single_gpu
```

Shifted packed count scanner:

```bash
./build/baseplaneDna2Bench 67108864 16 1 30 packed_word64_shifted_count 20260504 single_gpu
nsys profile --stats=true --force-overwrite=true -t cuda \
  -o build/seq_dna2_nsys_shifted \
  ./build/baseplaneDna2Bench 67108864 16 1 5 packed_word64_shifted_count 20260504 single_gpu
```

Long-sequence CUDA checks:

```bash
./build/baseplaneDna2Bench 1073741824 16 1 20 packed_word64_shifted_count 20260504 single_gpu
./build/baseplaneDna2Bench 1073741824 16 1 20 packed_word64_shifted_count 20260504 all_gpus
```

All-core CPU comparison:

```bash
./build/baseplaneDna2CpuBench 33554432 2 20260504 0 16 1
```

All benchmark-producing runs were serialized through the compare benchmark mutex
and the repository CUDA benchmark mutex.

## Profiler Evidence

For the old packed reference path on 67,108,864 bases, Nsight Systems reported:

- Kernel average: about 3.28 ms per launch.
- Device-to-host copy: 67.109 MB.
- Host-to-device copy: 16.777 MB.
- Device-to-host copy time: about 10.9 ms.
- Kernel attributes: 27 registers per thread, no local memory, no static shared
  memory, max 1024 threads per block.

This shows the old path was not limited by register spills or shared-memory
pressure. The loss was algorithmic and pipeline-facing: excess per-window scalar
work plus dense hit materialization.

For the shifted packed count path on the same 67,108,864-base input, Nsight
Systems reported:

- Kernel average: about 0.26 ms per launch.
- Device-to-host copy: one 8-byte hit count.
- Host-to-device copy: 16.777 MB.
- Device memset: one 8-byte counter reset per iteration.
- Kernel attributes: 32 registers per thread, no local memory, no static shared
  memory, max 1024 threads per block.

The remaining benchmark setup is mostly sequence upload and launch/sync
overhead. The measured kernel is now a compact streaming integer kernel.

## Benchmark Results

| Scenario | Implementation | Devices | Throughput |
| --- | ---: | ---: | ---: |
| 67,108,864 bases, motif 16 | old packed reference | 1 V100 | 21.3G windows/s |
| 67,108,864 bases, motif 16 | shifted packed count | 1 V100 | 301.9G windows/s |
| 67,108,864 bases, motif 16 | shifted packed count | 4 V100 | 601.7G windows/s |
| 67,108,864 bases, motif 16 | Highway CPU scan | 80 CPU threads | 11.8G windows/s |
| 268,435,456 bases, motif 16 | shifted packed count | 1 V100 | 288.1G windows/s |
| 268,435,456 bases, motif 16 | shifted packed count | 4 V100 | 955.7G windows/s |
| 1,073,741,824 bases, motif 16 | shifted packed count | 1 V100 | 294.7G windows/s |
| 1,073,741,824 bases, motif 16 | shifted packed count | 4 V100 | 1.128T windows/s |
| 1,073,741,824 bases, motif 16 | Highway CPU scan | 80 CPU threads | 13.3G windows/s |

Correctness checks:

```bash
cmake --build build --target baseplaneDna2CudaTest baseplaneDna2Bench baseplaneDna2CpuBench -j 4
./build/baseplaneDna2CudaTest
```

## Interpretation

The original CUDA result looked weak because it was doing too little useful GPU
work per byte moved and too much scalar work per candidate window. It treated
packed storage as something to unpack repeatedly inside each thread. That
defeated the central SequenceBits design sentence:

Packed words are how sequence lives in memory. Bitplanes are how sequence
becomes a warp-level biological decision.

The optimized packed path is stronger because it respects packed storage as the
streaming representation. For each start position, it builds the 32-base window
with at most two adjacent 64-bit packed words and a shift. Mismatch testing then
stays in packed form:

```cpp
uint64_t x = window.packed ^ motif.packed;
uint64_t y = x | (x >> 1);
uint64_t mismatch_fields = y & 0x5555555555555555ULL;
int mismatches = popcount(mismatch_fields & active_fields);
```

The benchmark also counts hits on device. That matters because random motif
scans usually produce sparse hits, so copying one byte per window is a poor
measurement of motif decision throughput.

Multi-GPU scaling is now real, but not perfect. At 1,073,741,824 bases, four
V100s deliver about 3.8x the single-GPU throughput. The remaining gap is from
host-side multi-device launch, synchronization, and per-device segmentation
overhead in the benchmark harness. This is acceptable for a primitive benchmark,
but a production scanner should keep sequence resident, batch many motifs, and
avoid one host launch train per small scan.

Nsight Compute was not used for the final interpretation because Nsight Systems
already isolated the dominant issue: the previous path was dominated by
algorithmic per-window work and dense hit copy, not low-level occupancy failure.
