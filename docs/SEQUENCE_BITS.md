# SequenceBits

## Purpose

SequenceBits provides Baseplane's first GPU-native regulatory sequence primitive.

Packed words are how sequence lives in memory. Bitplanes are how sequence becomes a warp-level biological decision.

## Encoding

A = 00
C = 01
G = 10
T = 11

This ordering is the format convention for the first `dna2` primitive.

## Two representations

## Packed word64

Storage / streaming layout.

One `uint64_t` stores 32 bases. Base `i` is stored at:

```text
(word >> (2 * i)) & 0x3
```

Base 0 is in the lowest two bits. The public active mask still has one bit per base; callers do not use the internal packed-field mask.

## Planes32 hi/lo

Warp-compute layout.

Two `uint32_t` words store a 32-base window:

```text
lo bit i = low bit of base i
hi bit i = high bit of base i
base_i = (((hi >> i) & 1) << 1) | ((lo >> i) & 1)
```

The representation is designed so one warp can hold one 32-base regulatory word.

## Conversion

`unpack_word64_to_planes32` expands each packed 2-bit field into the matching low and high bitplanes. `pack_planes32_to_word64` reconstructs each base from the hi/lo bitplanes and writes it back into the packed storage layout.

## Motif comparison

Planes compare with XOR / OR / POPCOUNT:

```cpp
uint32_t diff = ((a.lo ^ b.lo) | (a.hi ^ b.hi)) & active_mask;
int mismatches = popcount(diff);
```

Packed words first collapse each 2-bit field into one mismatch bit, then count through the public one-bit-per-base active mask.

## Reverse complement

Reverse complement applies both operations at once: reverse the active base order and complement each 2-bit base with `base ^ 0x3`. Lengths shorter than 32 produce zeroed inactive positions so unused bases do not leak into downstream comparisons.

## Kernel mapping

`lane = nucleotide`

`warp = 32-base regulatory word`

`ballot = bit-plane formation`

`popcount = mismatch count`

`warp_encode_base_lanes` assumes one lane owns one base. lane `i` contributes base `i`. The returned lo/hi planes encode the 32-base warp window.

The first scanner kernels are correctness primitives. `scan_motif_warp32_unpacked` maps one warp to one start position and forms planes with ballot. `scan_motif_word64_reference` scans packed storage with simple base extraction. Neither kernel is the final high-throughput scanner.

CUDA profiling notes and the first shifted packed scanner benchmark are recorded
in `docs/sequence_bits_cuda_profile.md`.

## CPU SIMD backend

`docs/sequence_simd_backend.md` describes the host-side scalar reference path
and optional Google Highway backend for the same packed word and bitplane
semantics. The CPU backend is deliberately kept API-compatible with future
accelerator implementations.

## Non-goals

No FASTA parser.
No full motif database support.
No PWM scoring yet.
No IUPAC ambiguity yet.
No CSPACK integration yet.
