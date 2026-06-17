# Baseplane Performance Contract

Baseplane benchmarks measure low-level biological bit primitives, not hidden
workflow convenience.

## Exact Scan

The exact scan primitive maps a packed DNA/RNA sequence and one `motif32_exact`
onto shifted `dna2_word64` windows. The public hot-path APIs are:

```cpp
scan_exact_count_cpu(sequence, motif, &host_count);
scan_exact_emit_cpu(sequence, motif, host_hits);
scan_exact_count_cuda(stream, sequence, motif, device_count);
scan_exact_emit_cuda(stream, sequence, motif, device_hits);
```

These APIs do not allocate. Callers provide the packed sequence view, output
counter or compact hit buffer, and CUDA stream when applicable.

## Plane Streams

The plane-stream primitive materializes resident split `dna2_planes32` words
from packed `dna2_word64` storage and derives predicate masks without dense
byte outputs:

```cpp
dna2_to_planes32_stream_cpu(sequence, planes);
dna2_to_planes32_stream_cuda(stream, sequence, planes);
planes32_stream_base_mask_*;
planes32_stream_gc_mask_*;
planes32_stream_cpg_start_mask_*;
```

The hot path is allocation-free. Callers own `lo_words`, `hi_words`, and
`uint32_t` mask output arrays. One output mask word represents 32 bases.

## Output Modes

Supported stable output modes:

- `count`: one hit counter.
- `compact emit`: sparse `motif_hit` records with explicit capacity accounting.
- `stream mask`: one `uint32_t` predicate mask per 32-base resident word.

Dense byte-per-window hit buffers are reference/debug outputs only. They should
not be used for throughput claims unless the benchmark is explicitly measuring
dense materialization.

## What To Record

When reporting performance, include:

- hardware and CUDA/compiler version
- build flags and CUDA architecture
- command line
- sequence length and motif length
- mismatch threshold and hit density
- kernel time and end-to-end time
- H2D and D2H bytes
- registers per thread and shared memory
- output mode and output capacity
- number of devices

Benchmark and profiler jobs must use the benchmark mutex on shared systems.

## Current Baseline

The current native tuning baseline is Tesla V100, `sm_70`. Exact count uses the
shifted packed-word scanner. Compact emit uses the same shifted packed-window
matching logic and writes sparse records with atomic rank assignment.

Plane streams use regular custom CUDA bitwise kernels. Tensor Cores are not
eligible because the work is integer bit extraction and Boolean predicate
materialization, not dense blocked matrix math. On 2026-06-17, one V100 with
CUDA 12.9.86 and `CMAKE_CUDA_ARCHITECTURES=70` measured 67,108,864 bases at
0.045 ms for packed-to-plane conversion and 0.034 ms for base, GC, and
CpG-start masks. The first limiter to re-check is HBM traffic plus launch
overhead.
