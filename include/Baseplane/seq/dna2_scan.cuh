#pragma once

#include <Baseplane/seq/dna2_scan.hh>

#if defined(__CUDACC__)
#include <cuda_runtime.h>

namespace baseplane::seq {

#define BASEPLANE_SEQ_DEVICE __device__ __forceinline__

BASEPLANE_SEQ_DEVICE dna2_planes32 warp_encode_base_lanes(std::uint8_t base, unsigned mask = 0xffffffffu) {
    dna2_planes32 p;
    p.lo = __ballot_sync(mask, (base & 0x1u) != 0u);
    p.hi = __ballot_sync(mask, (base & 0x2u) != 0u);
    return p;
}

#undef BASEPLANE_SEQ_DEVICE

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

__global__ void scan_motif_word64_shifted_emit(
    const std::uint64_t* packed_seq,
    int n_bases,
    motif32_exact motif,
    compact_motif_hit_buffer hits);

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

__global__ void dna2_to_planes32_stream_kernel(
    const std::uint64_t* packed_words,
    std::uint32_t* lo_words,
    std::uint32_t* hi_words,
    std::uint64_t n_words);

__global__ void planes32_stream_base_mask_kernel(
    const std::uint32_t* lo_words,
    const std::uint32_t* hi_words,
    std::uint8_t base_code,
    std::uint32_t* output_masks,
    std::uint64_t n_words);

__global__ void planes32_stream_gc_mask_kernel(
    const std::uint32_t* lo_words,
    const std::uint32_t* hi_words,
    std::uint32_t* output_masks,
    std::uint64_t n_words);

__global__ void planes32_stream_cpg_start_mask_kernel(
    const std::uint32_t* lo_words,
    const std::uint32_t* hi_words,
    std::uint32_t* output_masks,
    std::uint64_t n_words);

baseplane::status scan_exact_count_cuda(
    cudaStream_t stream,
    dna2_packed64_view sequence,
    motif32_exact motif,
    unsigned long long* device_count);

baseplane::status scan_exact_emit_cuda(
    cudaStream_t stream,
    dna2_packed64_view sequence,
    motif32_exact motif,
    compact_motif_hit_buffer device_hits);

baseplane::status dna2_to_planes32_stream_cuda(
    cudaStream_t stream,
    dna2_packed64_view sequence,
    dna2_planes32_stream_mutable_view output);

baseplane::status planes32_stream_base_mask_cuda(
    cudaStream_t stream,
    dna2_planes32_stream_view input,
    std::uint8_t base_code,
    dna2_mask32_stream_mutable_view output);

baseplane::status planes32_stream_gc_mask_cuda(
    cudaStream_t stream,
    dna2_planes32_stream_view input,
    dna2_mask32_stream_mutable_view output);

baseplane::status planes32_stream_cpg_start_mask_cuda(
    cudaStream_t stream,
    dna2_planes32_stream_view input,
    dna2_mask32_stream_mutable_view output);

} // namespace baseplane::seq

#endif
