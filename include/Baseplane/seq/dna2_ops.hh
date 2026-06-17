#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

#ifndef BASEPLANE_ENABLE_CUDA
#define BASEPLANE_ENABLE_CUDA 0
#endif

#ifndef BASEPLANE_ENABLE_HIGHWAY
#define BASEPLANE_ENABLE_HIGHWAY 0
#endif

#if defined(__CUDACC__)
#define BASEPLANE_DNA2_HD __host__ __device__ __forceinline__
#else
#define BASEPLANE_DNA2_HD inline
#endif

namespace baseplane::seq {

// Canonical 2-bit sequence encoding for packed DNA/RNA primitives:
// A = 00, C = 01, G = 10, T/U = 11. The ASCII packer below accepts uppercase
// A/C/G/T only; non-canonical bytes assert in debug builds and pack as A.
enum class dna2_base : std::uint8_t {
    A = 0,
    C = 1,
    G = 2,
    T = 3,
};

enum class dna2_backend : std::uint8_t {
    scalar = 0,
    highway_simd = 1,
    cuda_warp32 = 2,
};

struct dna2_word32 {
    std::uint32_t packed;
};

struct dna2_word64 {
    std::uint64_t packed;
};

struct dna2_planes32 {
    std::uint32_t lo;
    std::uint32_t hi;
};

struct dna2_planes64 {
    std::uint64_t lo;
    std::uint64_t hi;
};

struct dna2_inlplane32 {
    std::uint32_t planes;
};

struct dna2_inlplane64 {
    std::uint64_t planes;
};

struct dna2_window32 {
    dna2_planes32 planes;
    std::uint32_t valid_mask;
    std::uint64_t genomic_offset;
};

using dna2_storage_word = dna2_word64;
using dna2_warp_word = dna2_planes32;
using dna2_default_mask = std::uint32_t;

#if BASEPLANE_ENABLE_CUDA
using dna2_default_window = dna2_planes32;
inline constexpr dna2_backend dna2_default_backend = dna2_backend::cuda_warp32;
#elif BASEPLANE_ENABLE_HIGHWAY
using dna2_default_window = dna2_word64;
inline constexpr dna2_backend dna2_default_backend = dna2_backend::highway_simd;
#else
using dna2_default_window = dna2_word64;
inline constexpr dna2_backend dna2_default_backend = dna2_backend::scalar;
#endif

static_assert(std::is_same_v<dna2_storage_word, dna2_word64>);
static_assert(std::is_same_v<dna2_warp_word, dna2_planes32>);
static_assert(std::is_same_v<dna2_default_mask, std::uint32_t>);

namespace detail {

constexpr std::uint32_t all_bases_mask = 0xffffffffu;
constexpr std::uint32_t low16_bases_mask = 0x0000ffffu;
constexpr std::uint64_t packed_field_lo_mask = 0x5555555555555555ULL;

BASEPLANE_DNA2_HD std::uint32_t active_mask16_from_length(int length) {
    if (length <= 0) return 0u;
    if (length >= 16) return low16_bases_mask;
    return (1u << static_cast<unsigned int>(length)) - 1u;
}

BASEPLANE_DNA2_HD std::uint32_t active_mask_from_length(int length) {
    if (length <= 0) return 0u;
    if (length >= 32) return all_bases_mask;
    return (1u << static_cast<unsigned int>(length)) - 1u;
}

BASEPLANE_DNA2_HD std::uint64_t active_mask64_from_length(int length) {
    if (length <= 0) return 0ULL;
    if (length >= 64) return 0xffffffffffffffffULL;
    return (1ULL << static_cast<unsigned int>(length)) - 1ULL;
}

BASEPLANE_DNA2_HD int popcount32(std::uint32_t value) {
#if defined(__CUDA_ARCH__)
    return __popc(value);
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_popcount(value);
#else
    int count = 0;
    while (value != 0u) {
        count += static_cast<int>(value & 1u);
        value >>= 1u;
    }
    return count;
#endif
}

BASEPLANE_DNA2_HD int popcount64(std::uint64_t value) {
#if defined(__CUDA_ARCH__)
    return __popcll(static_cast<unsigned long long>(value));
#elif defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(static_cast<unsigned long long>(value));
#else
    int count = 0;
    while (value != 0u) {
        count += static_cast<int>(value & 1u);
        value >>= 1u;
    }
    return count;
#endif
}

BASEPLANE_DNA2_HD std::uint64_t spread_active_mask_to_packed_fields(std::uint32_t active_mask) {
    std::uint64_t bits = active_mask;
    bits = (bits | (bits << 16u)) & 0x0000ffff0000ffffULL;
    bits = (bits | (bits << 8u)) & 0x00ff00ff00ff00ffULL;
    bits = (bits | (bits << 4u)) & 0x0f0f0f0f0f0f0f0fULL;
    bits = (bits | (bits << 2u)) & 0x3333333333333333ULL;
    bits = (bits | (bits << 1u)) & packed_field_lo_mask;
    return bits;
}

BASEPLANE_DNA2_HD int word64_mismatches_packed_count_fields(
    dna2_word64 a,
    dna2_word64 b,
    std::uint64_t active_fields) {
    const std::uint64_t x = a.packed ^ b.packed;
    const std::uint64_t y = x | (x >> 1u);
    const std::uint64_t mismatch_fields = y & packed_field_lo_mask;
    return popcount64(mismatch_fields & active_fields);
}

} // namespace detail

BASEPLANE_DNA2_HD std::uint8_t make_base(char c) {
    return (c == 'C' || c == 'c') ? static_cast<std::uint8_t>(dna2_base::C)
        : (c == 'G' || c == 'g') ? static_cast<std::uint8_t>(dna2_base::G)
        : (c == 'T' || c == 't' || c == 'U' || c == 'u') ? static_cast<std::uint8_t>(dna2_base::T)
        : static_cast<std::uint8_t>(dna2_base::A);
}

BASEPLANE_DNA2_HD char base_to_char(std::uint8_t b) {
    const std::uint8_t base = b & 0x3u;
    return base == static_cast<std::uint8_t>(dna2_base::C) ? 'C'
        : base == static_cast<std::uint8_t>(dna2_base::G) ? 'G'
        : base == static_cast<std::uint8_t>(dna2_base::T) ? 'T'
        : 'A';
}

BASEPLANE_DNA2_HD std::uint8_t get_base(dna2_word32 w, int i) {
    const unsigned int slot = static_cast<unsigned int>(i) & 15u;
    return static_cast<std::uint8_t>((w.packed >> (slot * 2u)) & 0x3u);
}

BASEPLANE_DNA2_HD std::uint8_t get_base(dna2_word64 w, int i) {
    const unsigned int slot = static_cast<unsigned int>(i) & 31u;
    return static_cast<std::uint8_t>((w.packed >> (slot * 2u)) & 0x3ULL);
}

BASEPLANE_DNA2_HD void set_base(dna2_word32& w, int i, std::uint8_t b) {
    const unsigned int slot = static_cast<unsigned int>(i) & 15u;
    const unsigned int shift = slot * 2u;
    const std::uint32_t mask = 0x3u << shift;
    w.packed = (w.packed & ~mask) | ((static_cast<std::uint32_t>(b & 0x3u)) << shift);
}

BASEPLANE_DNA2_HD void set_base(dna2_word64& w, int i, std::uint8_t b) {
    const unsigned int slot = static_cast<unsigned int>(i) & 31u;
    const unsigned int shift = slot * 2u;
    const std::uint64_t mask = 0x3ULL << shift;
    w.packed = (w.packed & ~mask) | ((static_cast<std::uint64_t>(b & 0x3u)) << shift);
}

BASEPLANE_DNA2_HD dna2_planes32 unpack_word32_to_planes32(dna2_word32 w) {
    dna2_planes32 p{0u, 0u};
    for (int i = 0; i < 16; ++i) {
        const std::uint8_t base = get_base(w, i);
        p.lo |= static_cast<std::uint32_t>(base & 0x1u) << i;
        p.hi |= static_cast<std::uint32_t>((base >> 1u) & 0x1u) << i;
    }
    return p;
}

BASEPLANE_DNA2_HD dna2_planes32 unpack_word64_to_planes32(dna2_word64 w) {
    dna2_planes32 p{0u, 0u};
    for (int i = 0; i < 32; ++i) {
        const std::uint8_t base = get_base(w, i);
        p.lo |= static_cast<std::uint32_t>(base & 0x1u) << i;
        p.hi |= static_cast<std::uint32_t>((base >> 1u) & 0x1u) << i;
    }
    return p;
}

BASEPLANE_DNA2_HD dna2_word32 pack_planes32_to_word32(dna2_planes32 p) {
    dna2_word32 w{0u};
    for (int i = 0; i < 16; ++i) {
        const std::uint8_t base = static_cast<std::uint8_t>(
            ((((p.hi >> i) & 0x1u) << 1u) | ((p.lo >> i) & 0x1u)) & 0x3u);
        set_base(w, i, base);
    }
    return w;
}

BASEPLANE_DNA2_HD dna2_word64 pack_planes32_to_word64(dna2_planes32 p) {
    dna2_word64 w{0ULL};
    for (int i = 0; i < 32; ++i) {
        const std::uint8_t base = static_cast<std::uint8_t>(
            ((((p.hi >> i) & 0x1u) << 1u) | ((p.lo >> i) & 0x1u)) & 0x3u);
        set_base(w, i, base);
    }
    return w;
}

BASEPLANE_DNA2_HD dna2_planes32 inlplane32_to_planes32(dna2_inlplane32 w) {
    return dna2_planes32{
        static_cast<std::uint32_t>(w.planes & detail::low16_bases_mask),
        static_cast<std::uint32_t>((w.planes >> 16u) & detail::low16_bases_mask)
    };
}

BASEPLANE_DNA2_HD dna2_inlplane32 planes32_to_inlplane32(dna2_planes32 p) {
    return dna2_inlplane32{
        static_cast<std::uint32_t>((p.lo & detail::low16_bases_mask) | ((p.hi & detail::low16_bases_mask) << 16u))
    };
}

BASEPLANE_DNA2_HD dna2_planes32 inlplane64_to_planes32(dna2_inlplane64 w) {
    return dna2_planes32{
        static_cast<std::uint32_t>(w.planes),
        static_cast<std::uint32_t>(w.planes >> 32u)
    };
}

BASEPLANE_DNA2_HD dna2_inlplane64 planes32_to_inlplane64(dna2_planes32 p) {
    return dna2_inlplane64{static_cast<std::uint64_t>(p.lo) | (static_cast<std::uint64_t>(p.hi) << 32u)};
}

BASEPLANE_DNA2_HD dna2_inlplane32 word32_to_inlplane32(dna2_word32 w) {
    return planes32_to_inlplane32(unpack_word32_to_planes32(w));
}

BASEPLANE_DNA2_HD dna2_word32 inlplane32_to_word32(dna2_inlplane32 w) {
    return pack_planes32_to_word32(inlplane32_to_planes32(w));
}

BASEPLANE_DNA2_HD dna2_inlplane64 word64_to_inlplane64(dna2_word64 w) {
    return planes32_to_inlplane64(unpack_word64_to_planes32(w));
}

BASEPLANE_DNA2_HD dna2_word64 inlplane64_to_word64(dna2_inlplane64 w) {
    return pack_planes32_to_word64(inlplane64_to_planes32(w));
}

BASEPLANE_DNA2_HD std::uint32_t planes32_mismatch_mask(dna2_planes32 a, dna2_planes32 b, std::uint32_t active_mask) {
    return ((a.lo ^ b.lo) | (a.hi ^ b.hi)) & active_mask;
}

BASEPLANE_DNA2_HD int planes32_mismatches(dna2_planes32 a, dna2_planes32 b, std::uint32_t active_mask) {
    return detail::popcount32(planes32_mismatch_mask(a, b, active_mask));
}

BASEPLANE_DNA2_HD bool planes32_exact_match(dna2_planes32 a, dna2_planes32 b, std::uint32_t active_mask) {
    return planes32_mismatch_mask(a, b, active_mask) == 0u;
}

BASEPLANE_DNA2_HD std::uint64_t planes64_mismatch_mask(dna2_planes64 a, dna2_planes64 b, std::uint64_t active_mask) {
    return ((a.lo ^ b.lo) | (a.hi ^ b.hi)) & active_mask;
}

BASEPLANE_DNA2_HD int planes64_mismatches(dna2_planes64 a, dna2_planes64 b, std::uint64_t active_mask) {
    return detail::popcount64(planes64_mismatch_mask(a, b, active_mask));
}

BASEPLANE_DNA2_HD bool planes64_exact_match(dna2_planes64 a, dna2_planes64 b, std::uint64_t active_mask) {
    return planes64_mismatch_mask(a, b, active_mask) == 0ULL;
}

BASEPLANE_DNA2_HD std::uint64_t planes64_base_mask(dna2_planes64 planes, std::uint8_t code) {
    const std::uint64_t lo = planes.lo;
    const std::uint64_t hi = planes.hi;
    return code == 0u ? (~lo & ~hi)
        : code == 1u ? (lo & ~hi)
        : code == 2u ? (~lo & hi)
        : (lo & hi);
}

BASEPLANE_DNA2_HD std::uint64_t planes64_gc_mask(dna2_planes64 planes) {
    return planes.lo ^ planes.hi;
}

BASEPLANE_DNA2_HD std::uint64_t planes64_cpg_start_mask(dna2_planes64 planes) {
    const std::uint64_t c_mask = planes64_base_mask(planes, 1u);
    const std::uint64_t g_mask = planes64_base_mask(planes, 2u);
    return c_mask & (g_mask >> 1u) & 0x7fffffffffffffffULL;
}

BASEPLANE_DNA2_HD std::uint32_t inlplane32_mismatch_mask(dna2_inlplane32 a, dna2_inlplane32 b, std::uint32_t active_mask_16bases) {
    const std::uint32_t x = a.planes ^ b.planes;
    return ((x & detail::low16_bases_mask) | (x >> 16u)) & active_mask_16bases & detail::low16_bases_mask;
}

BASEPLANE_DNA2_HD int inlplane32_mismatches(dna2_inlplane32 a, dna2_inlplane32 b, std::uint32_t active_mask_16bases) {
    return detail::popcount32(inlplane32_mismatch_mask(a, b, active_mask_16bases));
}

BASEPLANE_DNA2_HD bool inlplane32_exact_match(dna2_inlplane32 a, dna2_inlplane32 b, std::uint32_t active_mask_16bases) {
    return inlplane32_mismatch_mask(a, b, active_mask_16bases) == 0u;
}

BASEPLANE_DNA2_HD std::uint32_t inlplane64_mismatch_mask(dna2_inlplane64 a, dna2_inlplane64 b, std::uint32_t active_mask_32bases) {
    const std::uint64_t x = a.planes ^ b.planes;
    return static_cast<std::uint32_t>((x | (x >> 32u)) & static_cast<std::uint64_t>(active_mask_32bases));
}

BASEPLANE_DNA2_HD int inlplane64_mismatches(dna2_inlplane64 a, dna2_inlplane64 b, std::uint32_t active_mask_32bases) {
    return detail::popcount32(inlplane64_mismatch_mask(a, b, active_mask_32bases));
}

BASEPLANE_DNA2_HD bool inlplane64_exact_match(dna2_inlplane64 a, dna2_inlplane64 b, std::uint32_t active_mask_32bases) {
    return inlplane64_mismatch_mask(a, b, active_mask_32bases) == 0u;
}

BASEPLANE_DNA2_HD std::uint32_t word64_mismatch_mask(dna2_word64 a, dna2_word64 b, std::uint32_t active_mask_32bases) {
    const std::uint64_t x = a.packed ^ b.packed;
    const std::uint64_t y = x | (x >> 1u);
    const std::uint64_t mismatch_fields = y & detail::packed_field_lo_mask;
    std::uint32_t mask = 0u;
    for (int i = 0; i < 32; ++i) {
        mask |= static_cast<std::uint32_t>((mismatch_fields >> (2u * static_cast<unsigned int>(i))) & 0x1ULL) << i;
    }
    return mask & active_mask_32bases;
}

BASEPLANE_DNA2_HD int word64_mismatches(dna2_word64 a, dna2_word64 b, std::uint32_t active_mask_32bases) {
    return detail::popcount32(word64_mismatch_mask(a, b, active_mask_32bases));
}

BASEPLANE_DNA2_HD int word64_mismatches_packed_count(dna2_word64 a, dna2_word64 b, std::uint32_t active_mask_32bases) {
    const std::uint64_t active_fields = detail::spread_active_mask_to_packed_fields(active_mask_32bases);
    return detail::word64_mismatches_packed_count_fields(a, b, active_fields);
}

BASEPLANE_DNA2_HD std::uint32_t word32_mismatch_mask(dna2_word32 a, dna2_word32 b, std::uint32_t active_mask_16bases) {
    return word64_mismatch_mask(
        dna2_word64{static_cast<std::uint64_t>(a.packed)},
        dna2_word64{static_cast<std::uint64_t>(b.packed)},
        active_mask_16bases & detail::low16_bases_mask);
}

BASEPLANE_DNA2_HD int word32_mismatches(dna2_word32 a, dna2_word32 b, std::uint32_t active_mask_16bases) {
    return detail::popcount32(word32_mismatch_mask(a, b, active_mask_16bases));
}

BASEPLANE_DNA2_HD bool word32_exact_match(dna2_word32 a, dna2_word32 b, std::uint32_t active_mask_16bases) {
    return word32_mismatch_mask(a, b, active_mask_16bases) == 0u;
}

dna2_word64 dna2_pack_ascii_32(const char* bases, std::size_t n);
void dna2_unpack_ascii_32(dna2_word64 word, char* out, std::size_t n);

dna2_planes32 dna2_to_planes(dna2_word64 word);
dna2_word64 planes_to_dna2(dna2_planes32 planes);

std::uint32_t planes_base_mask(dna2_planes32 planes, char base);
std::uint32_t planes_gc_mask(dna2_planes32 planes);
std::uint32_t planes_cpg_start_mask(dna2_planes32 planes);

std::uint32_t dna2_hamming_distance(dna2_word64 a, dna2_word64 b, std::size_t n);

void dna2_pack_ascii_batch_scalar(
    const char* input,
    std::size_t stride,
    dna2_word64* output,
    std::size_t count,
    std::size_t n_per_seq);

void dna2_unpack_ascii_batch_scalar(
    const dna2_word64* input,
    char* output,
    std::size_t stride,
    std::size_t count,
    std::size_t n_per_seq);

void dna2_to_planes_batch_scalar(
    const dna2_word64* input,
    dna2_planes32* output,
    std::size_t count);

void planes_gc_mask_batch_scalar(
    const dna2_planes32* input,
    std::uint32_t* output_masks,
    std::size_t count);

void planes_cpg_start_mask_batch_scalar(
    const dna2_planes32* input,
    std::uint32_t* output_masks,
    std::size_t count);

void dna2_pack_ascii_batch_highway(
    const char* input,
    std::size_t stride,
    dna2_word64* output,
    std::size_t count,
    std::size_t n_per_seq);

void dna2_unpack_ascii_batch_highway(
    const dna2_word64* input,
    char* output,
    std::size_t stride,
    std::size_t count,
    std::size_t n_per_seq);

void dna2_to_planes_batch_highway(
    const dna2_word64* input,
    dna2_planes32* output,
    std::size_t count);

void planes_gc_mask_batch_highway(
    const dna2_planes32* input,
    std::uint32_t* output_masks,
    std::size_t count);

void planes_cpg_start_mask_batch_highway(
    const dna2_planes32* input,
    std::uint32_t* output_masks,
    std::size_t count);

void dna2_pack_ascii_batch(
    const char* input,
    std::size_t stride,
    dna2_word64* output,
    std::size_t count,
    std::size_t n_per_seq);

void dna2_unpack_ascii_batch(
    const dna2_word64* input,
    char* output,
    std::size_t stride,
    std::size_t count,
    std::size_t n_per_seq);

void dna2_to_planes_batch(
    const dna2_word64* input,
    dna2_planes32* output,
    std::size_t count);

void planes_gc_mask_batch(
    const dna2_planes32* input,
    std::uint32_t* output_masks,
    std::size_t count);

void planes_cpg_start_mask_batch(
    const dna2_planes32* input,
    std::uint32_t* output_masks,
    std::size_t count);

BASEPLANE_DNA2_HD dna2_word32 reverse_complement_word32(dna2_word32 w, int length) {
    const int n = length <= 0 ? 0 : (length > 16 ? 16 : length);
    dna2_word32 out{0u};
    for (int i = 0; i < n; ++i) {
        const std::uint8_t base = get_base(w, n - 1 - i) ^ 0x3u;
        set_base(out, i, base);
    }
    return out;
}

BASEPLANE_DNA2_HD dna2_word64 reverse_complement_word64(dna2_word64 w, int length) {
    const int n = length <= 0 ? 0 : (length > 32 ? 32 : length);
    dna2_word64 out{0ULL};
    for (int i = 0; i < n; ++i) {
        const std::uint8_t base = get_base(w, n - 1 - i) ^ 0x3u;
        set_base(out, i, base);
    }
    return out;
}

BASEPLANE_DNA2_HD dna2_inlplane32 reverse_complement_inlplane32(dna2_inlplane32 w, int length) {
    return word32_to_inlplane32(reverse_complement_word32(inlplane32_to_word32(w), length));
}

BASEPLANE_DNA2_HD dna2_planes32 reverse_complement_planes32(dna2_planes32 p, int length) {
    return unpack_word64_to_planes32(reverse_complement_word64(pack_planes32_to_word64(p), length));
}

BASEPLANE_DNA2_HD dna2_inlplane64 reverse_complement_inlplane64(dna2_inlplane64 w, int length) {
    return planes32_to_inlplane64(reverse_complement_planes32(inlplane64_to_planes32(w), length));
}

BASEPLANE_DNA2_HD dna2_planes64 reverse_complement_planes64(dna2_planes64 p, int length) {
    const int n = length <= 0 ? 0 : (length > 64 ? 64 : length);
    dna2_planes64 out{0ULL, 0ULL};
    for (int i = 0; i < n; ++i) {
        const int source = n - 1 - i;
        const std::uint8_t base = static_cast<std::uint8_t>(
            ((((p.hi >> static_cast<unsigned int>(source)) & 0x1ULL) << 1u)
                | ((p.lo >> static_cast<unsigned int>(source)) & 0x1ULL)) ^ 0x3u);
        out.lo |= static_cast<std::uint64_t>(base & 0x1u) << static_cast<unsigned int>(i);
        out.hi |= static_cast<std::uint64_t>((base >> 1u) & 0x1u) << static_cast<unsigned int>(i);
    }
    return out;
}

} // namespace baseplane::seq

#undef BASEPLANE_DNA2_HD
