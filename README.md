# Baseplane

**Baseplane grounds biology in sequence.**

Baseplane is the sequence-grounding layer of a larger biological-compute
program. Its atomic scientific unit is the base: the irreducible unit of exact
sequence identity, variation, provenance, ordering, strand, and sequence truth.
That does not require every computation to operate one nucleotide at a time.
Baseplane may reason through motifs, intervals, genes, transcripts, regulatory
structures, haplotypes, learned features, multiresolution regions, long-range
relations, or structures not yet named. The invariant is that higher
sequence-derived structure should retain a principled path to its supporting
sequence and, where scientifically possible, to exact bases.

The project investigates a major hypothesis:

> Genomic span need not equal computational context cost, if exploitable
> biological organization can make relevant sequence computationally close
> while irrelevant sequence remains cheap.

This is not a proven result. Nor may efficiency come from deciding in advance
what biology is allowed to matter. Known biology should be usable; unknown
biology must remain discoverable.

The ecosystem distinction is:

- **Baseplane is sequence.**
- **Cellerator is omics.**
- **GlassHelix is dynamics.**
- **CellShard is physical scale, persistence, distribution, and delivery.**

Or, compactly: Baseplane grounds biology in sequence. Cellerator makes
biological state calculable. GlassHelix models how that state evolves.
CellShard makes the computation scale.

See [Scientific Foundations](docs/FOUNDATIONS.md) for the project constitution,
scientific hypotheses, ownership boundaries, unresolved architectural
questions, and proof obligations.

## Implemented today

The current library is a durable exact-sequence foundation, not the complete
scientific model of Baseplane. It provides compact canonical DNA/RNA
representations; explicit validity and tail semantics; local/global chunk
coordinates, ownership, and halos; strand-aware exact and allowed-base
predicates; compact count, mask, and event contracts; bounded backend-neutral
predicate descriptions; scalar reference behavior; optional Highway SIMD; and
optional CUDA kernels with explicit storage and stream ownership.

These capabilities are described in:

- [Current exact-sequence substrate](docs/BITOP_ARCHITECTURE.md)
- [Sequence representation and operations](docs/SEQUENCE_BITS.md)
- [Performance contract](docs/PERFORMANCE_CONTRACT.md)
- [Exact-count baseline evidence](docs/bitop_baseline_evidence.md)

The present motif grammar, 32-base windows, chunking scheme, and backend
lowerings are implementation mechanisms. They do not define the eventual
architecture or limit Baseplane to motif scanning.

## Build and use the current library

CUDA is preferred for current hot paths but remains optional:

```bash
cmake -S . -B build -DBASEPLANE_ENABLE_CUDA=ON
cmake --build build --target baseplaneDna2Test baseplaneDna2CudaTest baseplaneDna2Bench baseplaneDna2CpuBench -j 4
```

CPU-only build:

```bash
cmake -S . -B build-cpu -DBASEPLANE_ENABLE_CUDA=OFF
cmake --build build-cpu --target baseplaneDna2Test baseplaneBitOpContractTest baseplaneDna2ValidityTest -j 4
```

The CPU-safe public include is:

```cpp
#include <Baseplane/dna2.hh>
```

CUDA sequence operations are exposed through:

```cpp
#include <Baseplane/seq/dna2.cuh>
```

The installable CMake target is:

```cmake
target_link_libraries(your_target PRIVATE Baseplane::seq)
```

Baseplane is independently buildable. Cellerator may consume sequence-derived
objects without taking ownership of their sequence provenance; CellShard may
store and deliver them without acquiring their scientific semantics.
