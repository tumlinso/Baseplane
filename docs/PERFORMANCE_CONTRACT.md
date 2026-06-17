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

## Output Modes

Supported stable output modes:

- `count`: one hit counter.
- `compact emit`: sparse `motif_hit` records with explicit capacity accounting.

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
