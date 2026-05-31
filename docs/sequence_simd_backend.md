# Sequence SIMD Backend

This layer defines CPU-side bit primitives for canonical packed DNA/RNA sequence
work. It is intentionally small: ASCII bases enter as short canonical chunks,
then kernels operate on packed 2-bit words and bitplanes rather than generic
strings.

## Packed 2-Bit Words

`dna2_word` stores up to 32 bases in one `uint64_t`, using:

- `A = 00`
- `C = 01`
- `G = 10`
- `T = 11`

Base `i` occupies bits `2*i` and `2*i + 1`. The ASCII packer currently accepts
uppercase `A/C/G/T`; non-canonical bytes assert in debug builds and are treated
as `A` in release builds. Ambiguous IUPAC bases are not represented in this
primitive layer.

Packed words make fixed-width sequence chunks cheap to move, compare, and stage
for later CPU SIMD or accelerator code. Unused positions in chunks shorter than
32 bases remain zeroed.

## Bitplanes

`dna2_planes` splits one packed word into two position masks:

- `lo`: low bit for each base position
- `hi`: high bit for each base position

With the canonical encoding, biologically meaningful masks are simple bitwise
operations:

- `C/G` mask: `lo ^ hi`
- `C` mask: `lo & ~hi`
- `G` mask: `~lo & hi`
- `CpG` start mask: `C(i) & G(i + 1)`

These masks are the CPU-side counterpart of future CUDA warp-level sequence
primitives. Both backends should preserve these semantic operations rather than
lowering sequence work to generic byte strings.

## Backends

The scalar implementation is the reference path and is always compiled. It
defines the public single-word operations plus scalar batch kernels.

When `BASEPLANE_ENABLE_HIGHWAY=ON`, CMake uses either
`BASEPLANE_HIGHWAY_SOURCE_DIR` or an installed `hwy` package, compiles
`dna2_highway.cpp`, links `Baseplane::seq` against `hwy`, and defines
`BASEPLANE_ENABLE_HIGHWAY=1`.
Public batch wrappers dispatch to the Highway backend in that build. The Highway
path uses mask extraction/materialization for full 32-base ASCII pack/unpack and
vector integer operations for packed-word-to-plane conversion plus GC/CpG masks.
With the option disabled, the same wrappers call scalar batch kernels and
Highway is not required.

Highway types stay inside `dna2_highway.cpp`; public headers expose only
Baseplane sequence types. This keeps room for later AVX2, AVX-512, NEON, or
CUDA-specialized implementations without changing the public API.
