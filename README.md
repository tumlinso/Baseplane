# Baseplane

Baseplane is a low-level sequence-computing library for compact DNA/RNA bit
representations. It provides a small backend-aware foundation for fast sequence
work: 2-bit nucleotide packing, bit-plane transforms, reverse complements,
mismatch counting, motif-window operations, and CPU/SIMD/CUDA kernels.

Baseplane is not a genomics framework or file-format library. It does not own
FASTA parsing, genome annotation, motif databases, PWM scoring, IUPAC ambiguity,
or storage/runtime formats. Higher-level tools should build those concerns on
top of Baseplane primitives.

## Build

CUDA is preferred for hot-path sequence scanning, but optional:

```bash
cmake -S . -B build -DBASEPLANE_ENABLE_CUDA=ON
cmake --build build --target baseplaneDna2Test baseplaneDna2CudaTest baseplaneDna2Bench baseplaneDna2CpuBench -j 4
```

CPU-only fallback:

```bash
cmake -S . -B build-cpu -DBASEPLANE_ENABLE_CUDA=OFF
cmake --build build-cpu --target baseplaneDna2Test -j 4
```

## Public Surface

The first public primitive is `baseplane::seq::dna2` under:

```cpp
#include <Baseplane/seq/dna2.cuh>
```

The CMake target is:

```cmake
target_link_libraries(your_target PRIVATE Baseplane::seq)
```
