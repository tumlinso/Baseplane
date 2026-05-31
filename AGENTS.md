# Baseplane Agent Guide

## Scope

Baseplane owns compact sequence bit primitives and backend-aware kernels for
DNA/RNA sequence computation. Keep it sharp and low level:

- 2-bit nucleotide packing and unpacking.
- Bit-plane transforms and masks.
- Reverse complements.
- Mismatch counting and motif-window operations.
- CPU scalar, optional SIMD, and CUDA kernels.

Baseplane is not a genomics framework or file-format library. Do not add FASTA
parsing, genome annotation ownership graphs, motif databases, PWM scoring,
IUPAC ambiguity expansion, CSPACK integration, CellShard storage behavior, or
Cellerator model/workflow policy unless project direction changes explicitly.

## CUDA Policy

CUDA is the preferred hot path, but it must remain optional at configure time.
CPU scalar primitives should build without a CUDA compiler. CUDA kernels should
favor packed device-resident words, shifted motif windows, bitwise mismatch
reduction, and device-side counting.

The native tuning target for this workspace is Volta `sm_70` on the 4x Tesla
V100 host. Keep CUDA builds narrow during tuning and preserve benchmark notes
for custom kernels. Tensor Cores are not applicable to the current integer and
bitwise sequence primitives.

## Style

Use C++17/CUDA17, 4-space indentation, explicit pointer-plus-size or contiguous
buffer interfaces for hot paths, and narrow translation units for kernels that
may need profiling. Avoid adding abstraction layers that hide residency,
packing, launch, or memory-traffic costs.

Benchmark and profiler runs must be serialized with the repository benchmark
mutex. Record exact commands and hardware context when changing CUDA kernels or
benchmark claims.
