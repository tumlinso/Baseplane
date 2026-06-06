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

The core CPU-safe `dna2` vocabulary is available under:

```cpp
#include <Baseplane/dna2.hh>
```

CUDA warp-level primitives and kernels are layered on top under:

```cpp
#include <Baseplane/seq/dna2.cuh>
```

The CMake target is:

```cmake
target_link_libraries(your_target PRIVATE Baseplane::seq)
```

`dna2_default_window` is selected at compile time. CUDA-enabled builds default
to the warp-native `dna2_planes32` representation; non-CUDA builds default to
packed `dna2_word64`, with Highway/SIMD preferred over scalar when enabled.
