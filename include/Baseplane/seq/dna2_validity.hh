#pragma once

#include <Baseplane/seq/dna2_views.hh>

#include <cstdint>

namespace baseplane::seq {

inline constexpr std::uint64_t dna2_max_local_base_count = 0x7fffffffULL;

struct dna2_encoded_base {
    std::uint8_t code;
    bool valid;
};

// The sidecar is optional. A null validity_words pointer with zero words means
// every base inside packed.n_bases is valid. Tail bits are never valid.
struct dna2_packed64_valid_view {
    dna2_packed64_view packed;
    const std::uint64_t* validity_words;
    std::uint64_t n_validity_words;
};

struct dna2_planes32_valid_stream_view {
    dna2_planes32_stream_view planes;
    const std::uint32_t* validity_masks;
    std::uint64_t base_count;
};

struct dna2_mask32_valid_stream_view {
    dna2_mask32_stream_view masks;
    const std::uint32_t* validity_masks;
    std::uint64_t base_count;
};

// These numeric tokens are Baseplane-local metadata, not a frozen shared ABI.
// A higher layer remains responsible for mapping its genome/contig identities.
struct dna2_chunk_identity {
    std::uint64_t genome;
    std::uint64_t contig;
    std::uint64_t chunk;
};

struct dna2_chunk_coordinates {
    dna2_chunk_identity identity;
    std::uint64_t global_base_begin;
    std::uint32_t base_count;
    std::uint32_t owned_begin;
    std::uint32_t owned_end;
    std::uint32_t halo_left;
    std::uint32_t halo_right;
};

std::uint64_t dna2_packed64_word_count(std::uint64_t base_count) noexcept;
std::uint64_t dna2_validity64_word_count(std::uint64_t base_count) noexcept;
std::uint64_t dna2_stream32_word_count(std::uint64_t base_count) noexcept;

std::uint64_t dna2_tail_mask64(std::uint64_t base_count, std::uint64_t word_index) noexcept;
std::uint32_t dna2_tail_mask32(std::uint64_t base_count, std::uint64_t word_index) noexcept;

dna2_encoded_base dna2_encode_base_with_validity(char base) noexcept;

bool dna2_valid_view(const dna2_packed64_valid_view& view) noexcept;
std::uint64_t dna2_validity_word(
    const dna2_packed64_valid_view& view,
    std::uint64_t word_index) noexcept;
bool dna2_base_is_valid(
    const dna2_packed64_valid_view& view,
    std::uint64_t local_position) noexcept;
bool dna2_window_is_valid(
    const dna2_packed64_valid_view& view,
    std::uint64_t local_begin,
    std::uint64_t length) noexcept;

bool dna2_valid_view(const dna2_planes32_valid_stream_view& view) noexcept;
std::uint32_t dna2_valid_mask(
    const dna2_planes32_valid_stream_view& view,
    std::uint64_t word_index) noexcept;

bool dna2_valid_view(const dna2_mask32_valid_stream_view& view) noexcept;
std::uint32_t dna2_valid_mask(
    const dna2_mask32_valid_stream_view& view,
    std::uint64_t word_index) noexcept;
std::uint32_t dna2_effective_mask_word(
    const dna2_mask32_valid_stream_view& view,
    std::uint64_t word_index) noexcept;

bool dna2_valid_chunk_coordinates(const dna2_chunk_coordinates& chunk) noexcept;
bool dna2_window_fits_chunk(
    const dna2_chunk_coordinates& chunk,
    std::uint32_t local_begin,
    std::uint32_t length) noexcept;
bool dna2_anchor_is_owned(
    const dna2_chunk_coordinates& chunk,
    std::uint32_t local_position) noexcept;
bool dna2_local_to_global(
    const dna2_chunk_coordinates& chunk,
    std::uint32_t local_position,
    std::uint64_t* global_position) noexcept;

} // namespace baseplane::seq
