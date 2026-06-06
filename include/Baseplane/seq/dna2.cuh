#pragma once

#include <Baseplane/dna2.hh>

#if defined(__CUDACC__)
#include <cuda_runtime.h>

#define BASEPLANE_SEQ_DEVICE __device__ __forceinline__

namespace baseplane::seq {

BASEPLANE_SEQ_DEVICE dna2_planes32 warp_encode_base_lanes(std::uint8_t base, unsigned mask = 0xffffffffu) {
    dna2_planes32 p;
    p.lo = __ballot_sync(mask, (base & 0x1u) != 0u);
    p.hi = __ballot_sync(mask, (base & 0x2u) != 0u);
    return p;
}

__global__ void scan_motif_warp32_unpacked(
    const std::uint8_t* seq_bases,
    int n_bases,
    dna2_planes32 motif,
    int motif_len,
    int max_mismatches,
    std::uint8_t* hits);

__global__ void convert_word64_to_planes32_warp(
    const std::uint64_t* packed_words,
    dna2_planes32* planes,
    int word_count);

__global__ void scan_motif_word64_reference(
    const std::uint64_t* packed_seq,
    int n_bases,
    dna2_word64 motif_word,
    int motif_len,
    int max_mismatches,
    std::uint8_t* hits);

__global__ void scan_motif_word64_shifted_count(
    const std::uint64_t* packed_seq,
    int n_bases,
    dna2_word64 motif_word,
    int motif_len,
    int max_mismatches,
    unsigned long long* hit_count);

__global__ void scan_motif_word64_aligned_count(
    const std::uint64_t* packed_words,
    int word_count,
    dna2_word64 motif_word,
    int motif_len,
    int max_mismatches,
    unsigned long long* hit_count);

__global__ void scan_motif_inlplane64_aligned_count(
    const std::uint64_t* inlplanes,
    int word_count,
    dna2_inlplane64 motif,
    int motif_len,
    int max_mismatches,
    unsigned long long* hit_count);

} // namespace baseplane::seq

#undef BASEPLANE_SEQ_DEVICE
#endif
