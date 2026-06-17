# SequenceBits

## Purpose

SequenceBits provides Baseplane's first GPU-native regulatory sequence primitive.

Packed words are how sequence lives in memory. Bitplanes are how sequence becomes a warp-level biological decision.

The CPU-safe representation and alias vocabulary lives in
`#include <Baseplane/dna2.hh>`. CUDA warp-level operations live in
`#include <Baseplane/seq/dna2.cuh>` and build on that vocabulary.
The split public headers under `include/Baseplane/seq/` expose the same surface
by role: `dna2_ops.hh` for representation operations, `dna2_views.hh` for
pointer/size views, `dna2_motif.hh` for motif and compact-hit records, and
`dna2_scan.hh` / `dna2_scan.cuh` for scan APIs.

## Encoding

A = 00
C = 01
G = 10
T = 11

This ordering is the format convention for the first `dna2` primitive.

## Representations

## Packed words

Storage / streaming layout.

`dna2_word32` stores 16 bases in one `uint32_t`; `dna2_word64` stores 32 bases
in one `uint64_t`. Base `i` is stored at:

```text
(word >> (2 * i)) & 0x3
```

Base 0 is in the lowest two bits. The public active mask still has one bit per base; callers do not use the internal packed-field mask.

## Split planes

Warp-compute layout.

`dna2_planes32` stores a 32-base window in two `uint32_t` planes.
`dna2_planes64` stores a 64-base window in two `uint64_t` planes.

```text
lo bit i = low bit of base i
hi bit i = high bit of base i
base_i = (((hi >> i) & 1) << 1) | ((lo >> i) & 1)
```

The `planes32` representation is designed so one warp can hold one 32-base
regulatory word.

Future CUDA-facing work should prefer `dna2_warp_word` or
`dna2_default_window`, which resolves to `dna2_planes32` when CUDA is enabled.

## Inline planes

Memory layout for loading a split-plane compute window as one scalar word.

`dna2_inlplane32` stores 16 bases:

```text
bits  0..15 = lo plane
bits 16..31 = hi plane
```

`dna2_inlplane64` stores 32 bases:

```text
bits  0..31 = lo plane
bits 32..63 = hi plane
```

These forms recast to `dna2_planes32` with shifts and masks, then use the same
XOR / OR / POPCOUNT mismatch logic as split planes.

`dna2_word32`, `dna2_planes64`, and `dna2_inlplane32/64` are retained alternate
representations. Use them when their storage or transport shape is intentional;
do not treat them as the default for new CUDA warp-level primitives.

## Conversion

`unpack_word64_to_planes32` expands each packed 2-bit field into the matching low and high bitplanes. `pack_planes32_to_word64` reconstructs each base from the hi/lo bitplanes and writes it back into the packed storage layout. `planes32_to_inlplane64` serializes the same planes as one inline-plane word; `inlplane64_to_planes32` restores the split-plane view.

## Plane streams

Resident plane streams keep the split layout as two caller-owned arrays:

```cpp
dna2_planes32_stream_view stream{
    lo_words,
    hi_words,
    n_words
};
```

Mutable stream outputs and mask outputs are explicit:

```cpp
dna2_planes32_stream_mutable_view out{lo_words, hi_words, n_words};
dna2_mask32_stream_mutable_view masks{mask_words, n_words};
```

The stable allocation-free APIs are:

```cpp
dna2_to_planes32_stream_cpu(sequence, out);
planes32_stream_base_mask_cpu(stream, base_code, masks);
planes32_stream_gc_mask_cpu(stream, masks);
planes32_stream_cpg_start_mask_cpu(stream, masks);
```

CUDA builds add stream-explicit equivalents:

```cpp
dna2_to_planes32_stream_cuda(cuda_stream, sequence, out);
planes32_stream_base_mask_cuda(cuda_stream, stream, base_code, masks);
planes32_stream_gc_mask_cuda(cuda_stream, stream, masks);
planes32_stream_cpg_start_mask_cuda(cuda_stream, stream, masks);
```

Each predicate output is one `uint32_t` mask per 32-base word. Conversion uses
`dna2_packed64_view::n_bases` to decide the required packed word count. Mask
stream APIs operate over `n_words`; callers that care about the final partial
word should apply their own active mask. CpG-start masks include starts at lane
31 when the next stream word begins with G.

## Motif comparison

Planes compare with XOR / OR / POPCOUNT:

```cpp
uint32_t diff = ((a.lo ^ b.lo) | (a.hi ^ b.hi)) & active_mask;
int mismatches = popcount(diff);
```

Packed words first collapse each 2-bit field into one mismatch bit, then count through the public one-bit-per-base active mask.

## Exact scan API

The stable exact motif scan surface is allocation-free:

```cpp
dna2_packed64_view sequence{
    words,
    n_bases,
    n_words
};
motif32_exact motif = make_motif32_exact(motif_word, length, max_mismatches, motif_id);

scan_exact_count_cpu(sequence, motif, &host_count);
scan_exact_emit_cpu(sequence, motif, compact_motif_hit_buffer{hits, capacity, &written, &dropped});
```

CUDA builds add stream-explicit wrappers:

```cpp
scan_exact_count_cuda(stream, sequence, motif, device_count);
scan_exact_emit_cuda(stream, sequence, motif, compact_motif_hit_buffer{
    device_hits,
    capacity,
    device_records_written,
    device_records_dropped
});
```

Count-only output writes one counter. Compact emit writes `motif_hit` records
until capacity is reached, increments `records_written` for every logical hit,
and increments `records_dropped` for overflow. Dense byte-per-window output is
kept as a reference/debug path, not the default public scan output.

## Reverse complement

Reverse complement applies both operations at once: reverse the active base order and complement each 2-bit base with `base ^ 0x3`. Lengths shorter than 32 produce zeroed inactive positions so unused bases do not leak into downstream comparisons.

## Kernel mapping

`lane = nucleotide`

`warp = 32-base regulatory word`

`ballot = bit-plane formation`

`popcount = mismatch count`

`warp_encode_base_lanes` assumes one lane owns one base. lane `i` contributes base `i`. The returned lo/hi planes encode the 32-base warp window.

The first scanner kernels are correctness primitives. `scan_motif_warp32_unpacked` maps one warp to one start position and forms planes with ballot. `scan_motif_word64_reference` scans packed storage with simple base extraction. The stable exact count API wraps the shifted packed scanner because fixed-width exact/Hamming scans are currently best represented as shifted `dna2_word64` windows.

CUDA profiling notes and the first shifted packed scanner benchmark are recorded
in `docs/sequence_bits_cuda_profile.md`.

The first resident plane-stream benchmark was run on 2026-06-17 on one
Tesla V100-SXM2-16GB with CUDA 12.9.86 and `CMAKE_CUDA_ARCHITECTURES=70`.
Commands:

```bash
./build/baseplaneDna2Bench 67108864 16 1 30 planes32_stream_convert 20260617 single_gpu
./build/baseplaneDna2Bench 67108864 16 1 30 planes32_stream_base_mask 20260617 single_gpu
./build/baseplaneDna2Bench 67108864 16 1 30 planes32_stream_gc_mask 20260617 single_gpu
./build/baseplaneDna2Bench 67108864 16 1 30 planes32_stream_cpg_start_mask 20260617 single_gpu
```

Results for 67,108,864 bases / 2,097,152 stream words: convert 0.045 ms,
base mask 0.034 ms, GC mask 0.034 ms, and CpG-start mask 0.034 ms. The stream
kernels used 16-18 registers per thread, 0 local bytes, and 0 static shared
bytes. H2D was 16,777,224 bytes including the existing packed sentinel; D2H was
16,777,216 bytes for conversion and 8,388,608 bytes for each mask output.

## CPU SIMD backend

`docs/sequence_simd_backend.md` describes the host-side scalar reference path
and optional Google Highway backend for the same packed word and bitplane
semantics. The CPU backend is deliberately kept API-compatible with future
accelerator implementations.

Backend preference is static at compile time: CUDA warp-native first, then
Highway/SIMD, then scalar. Benchmarks guide future priority changes manually;
they are not run during configure.

## Non-goals

No FASTA parser.
No full motif database support.
No PWM scoring yet.
No IUPAC ambiguity yet.
No CSPACK integration yet.
