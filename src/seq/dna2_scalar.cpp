#include <Baseplane/dna2.hh>

#include <cassert>

#ifndef BASEPLANE_ENABLE_HIGHWAY
#define BASEPLANE_ENABLE_HIGHWAY 0
#endif

namespace baseplane::seq {
namespace {

constexpr std::uint64_t packed_lo_mask = 0x5555555555555555ULL;
constexpr std::uint64_t all_base_positions = 0xffffffffULL;

std::uint32_t active_mask(std::size_t n) {
    if (n == 0u) return 0u;
    if (n >= 32u) return 0xffffffffu;
    return (1u << static_cast<unsigned int>(n)) - 1u;
}

std::uint8_t pack_ascii_base(char base) {
#ifndef NDEBUG
    assert(base == 'A' || base == 'C' || base == 'G' || base == 'T');
#endif
    return base == 'C' ? 1u : base == 'G' ? 2u : base == 'T' ? 3u : 0u;
}

std::uint64_t required_word_count(std::uint64_t n_bases) {
    return (n_bases + 31ULL) >> 5u;
}

bool valid_scan_input(dna2_packed64_view sequence, motif32_exact motif) {
    if (motif.length == 0u || motif.length > 32u) return false;
    if (sequence.n_bases > 0u && sequence.words == nullptr) return false;
    if (sequence.n_words < required_word_count(sequence.n_bases)) return false;
    return true;
}

bool valid_planes_stream_output(dna2_planes32_stream_mutable_view output, std::uint64_t n_words) {
    if (output.n_words < n_words) return false;
    if (n_words > 0u && (output.lo_words == nullptr || output.hi_words == nullptr)) return false;
    return true;
}

bool valid_planes_stream_input(dna2_planes32_stream_view input) {
    if (input.n_words > 0u && (input.lo_words == nullptr || input.hi_words == nullptr)) return false;
    return true;
}

bool valid_mask_stream_output(dna2_mask32_stream_mutable_view output, std::uint64_t n_words) {
    if (output.n_words < n_words) return false;
    if (n_words > 0u && output.masks == nullptr) return false;
    return true;
}

std::uint64_t shifted_window_word64_host(const std::uint64_t* packed_seq, std::uint64_t n_words, std::uint64_t start) {
    const std::uint64_t word_index = start >> 5u;
    const unsigned int shift = static_cast<unsigned int>(start & 31ULL) * 2u;
    const std::uint64_t lo = packed_seq[word_index];
    if (shift == 0u) return lo;
    const std::uint64_t hi = (word_index + 1ULL) < n_words ? packed_seq[word_index + 1ULL] : 0ULL;
    return (lo >> shift) | (hi << (64u - shift));
}

std::uint64_t compact_even_bits(std::uint64_t bits) {
    bits &= packed_lo_mask;
    bits = (bits | (bits >> 1u)) & 0x3333333333333333ULL;
    bits = (bits | (bits >> 2u)) & 0x0f0f0f0f0f0f0f0fULL;
    bits = (bits | (bits >> 4u)) & 0x00ff00ff00ff00ffULL;
    bits = (bits | (bits >> 8u)) & 0x0000ffff0000ffffULL;
    bits = (bits | (bits >> 16u)) & 0x00000000ffffffffULL;
    return bits;
}

std::uint64_t spread_bits32(std::uint64_t bits) {
    bits &= all_base_positions;
    bits = (bits | (bits << 16u)) & 0x0000ffff0000ffffULL;
    bits = (bits | (bits << 8u)) & 0x00ff00ff00ff00ffULL;
    bits = (bits | (bits << 4u)) & 0x0f0f0f0f0f0f0f0fULL;
    bits = (bits | (bits << 2u)) & 0x3333333333333333ULL;
    bits = (bits | (bits << 1u)) & packed_lo_mask;
    return bits;
}

std::uint32_t base_mask_from_code(dna2_planes32 planes, std::uint8_t code) {
    const std::uint64_t lo = planes.lo & all_base_positions;
    const std::uint64_t hi = planes.hi & all_base_positions;
    const std::uint64_t mask = code == 0u ? (~lo & ~hi)
        : code == 1u ? (lo & ~hi)
        : code == 2u ? (~lo & hi)
        : (lo & hi);
    return static_cast<std::uint32_t>(mask & all_base_positions);
}

} // namespace

dna2_word64 dna2_pack_ascii_32(const char* bases, std::size_t n) {
    assert(n <= 32u);
    assert(bases != nullptr || n == 0u);
    dna2_word64 out{0ULL};
    const std::size_t limit = n > 32u ? 32u : n;
    for (std::size_t i = 0u; i < limit; ++i) {
        out.packed |= static_cast<std::uint64_t>(pack_ascii_base(bases[i])) << (2u * static_cast<unsigned int>(i));
    }
    return out;
}

void dna2_unpack_ascii_32(dna2_word64 word, char* out, std::size_t n) {
    assert(n <= 32u);
    assert(out != nullptr || n == 0u);
    const std::size_t limit = n > 32u ? 32u : n;
    for (std::size_t i = 0u; i < limit; ++i) {
        const std::uint8_t base = static_cast<std::uint8_t>((word.packed >> (2u * static_cast<unsigned int>(i))) & 0x3ULL);
        out[i] = base_to_char(base);
    }
}

dna2_planes32 dna2_to_planes(dna2_word64 word) {
    return dna2_planes32{
        static_cast<std::uint32_t>(compact_even_bits(word.packed)),
        static_cast<std::uint32_t>(compact_even_bits(word.packed >> 1u))
    };
}

dna2_word64 planes_to_dna2(dna2_planes32 planes) {
    return dna2_word64{spread_bits32(planes.lo) | (spread_bits32(planes.hi) << 1u)};
}

std::uint32_t planes_base_mask(dna2_planes32 planes, char base) {
    return base_mask_from_code(planes, pack_ascii_base(base));
}

std::uint32_t planes_gc_mask(dna2_planes32 planes) {
    return static_cast<std::uint32_t>((planes.lo ^ planes.hi) & all_base_positions);
}

std::uint32_t planes_cpg_start_mask(dna2_planes32 planes) {
    const std::uint32_t c_mask = base_mask_from_code(planes, 1u);
    const std::uint32_t g_mask = base_mask_from_code(planes, 2u);
    return (c_mask & (g_mask >> 1u)) & 0x7fffffffu;
}

std::uint32_t dna2_hamming_distance(dna2_word64 a, dna2_word64 b, std::size_t n) {
    const std::uint32_t mask = word64_mismatch_mask(dna2_word64{a.packed}, dna2_word64{b.packed}, active_mask(n));
    return static_cast<std::uint32_t>(detail::popcount32(mask));
}

baseplane::status scan_exact_count_cpu(
    dna2_packed64_view sequence,
    motif32_exact motif,
    std::uint64_t* host_count) {
    if (host_count == nullptr || !valid_scan_input(sequence, motif)) {
        return baseplane::invalid_argument_status();
    }

    *host_count = 0ULL;
    if (sequence.n_bases < motif.length) {
        return baseplane::ok_status();
    }

    const std::uint64_t windows = sequence.n_bases - motif.length + 1ULL;
    const dna2_word64 motif_word{motif.packed};
    std::uint64_t hits = 0ULL;
    for (std::uint64_t start = 0ULL; start < windows; ++start) {
        const dna2_word64 window{shifted_window_word64_host(sequence.words, sequence.n_words, start)};
        const int mismatches = detail::word64_mismatches_packed_count_fields(window, motif_word, motif.active_fields);
        hits += mismatches <= motif.max_mismatches ? 1ULL : 0ULL;
    }
    *host_count = hits;
    return baseplane::ok_status();
}

baseplane::status dna2_to_planes32_stream_cpu(
    dna2_packed64_view sequence,
    dna2_planes32_stream_mutable_view output) {
    const std::uint64_t n_words = required_word_count(sequence.n_bases);
    if ((sequence.n_bases > 0u && sequence.words == nullptr)
        || sequence.n_words < n_words
        || !valid_planes_stream_output(output, n_words)) {
        return baseplane::invalid_argument_status();
    }

    for (std::uint64_t i = 0u; i < n_words; ++i) {
        const dna2_planes32 planes = dna2_to_planes(dna2_word64{sequence.words[i]});
        output.lo_words[i] = planes.lo;
        output.hi_words[i] = planes.hi;
    }
    return baseplane::ok_status();
}

baseplane::status planes32_stream_base_mask_cpu(
    dna2_planes32_stream_view stream,
    std::uint8_t base_code,
    dna2_mask32_stream_mutable_view output) {
    if (base_code > 3u || !valid_planes_stream_input(stream) || !valid_mask_stream_output(output, stream.n_words)) {
        return baseplane::invalid_argument_status();
    }

    for (std::uint64_t i = 0u; i < stream.n_words; ++i) {
        output.masks[i] = base_mask_from_code(dna2_planes32{stream.lo_words[i], stream.hi_words[i]}, base_code);
    }
    return baseplane::ok_status();
}

baseplane::status planes32_stream_gc_mask_cpu(
    dna2_planes32_stream_view stream,
    dna2_mask32_stream_mutable_view output) {
    if (!valid_planes_stream_input(stream) || !valid_mask_stream_output(output, stream.n_words)) {
        return baseplane::invalid_argument_status();
    }

    for (std::uint64_t i = 0u; i < stream.n_words; ++i) {
        output.masks[i] = planes_gc_mask(dna2_planes32{stream.lo_words[i], stream.hi_words[i]});
    }
    return baseplane::ok_status();
}

baseplane::status planes32_stream_cpg_start_mask_cpu(
    dna2_planes32_stream_view stream,
    dna2_mask32_stream_mutable_view output) {
    if (!valid_planes_stream_input(stream) || !valid_mask_stream_output(output, stream.n_words)) {
        return baseplane::invalid_argument_status();
    }

    for (std::uint64_t i = 0u; i < stream.n_words; ++i) {
        const dna2_planes32 planes{stream.lo_words[i], stream.hi_words[i]};
        std::uint32_t mask = planes_cpg_start_mask(planes);
        if ((i + 1u) < stream.n_words) {
            const dna2_planes32 next{stream.lo_words[i + 1u], stream.hi_words[i + 1u]};
            const std::uint32_t c_bit31 = base_mask_from_code(planes, 1u) & 0x80000000u;
            const std::uint32_t next_g_bit0 = base_mask_from_code(next, 2u) & 0x00000001u;
            if (c_bit31 != 0u && next_g_bit0 != 0u) mask |= 0x80000000u;
        }
        output.masks[i] = mask;
    }
    return baseplane::ok_status();
}

baseplane::status scan_exact_emit_cpu(
    dna2_packed64_view sequence,
    motif32_exact motif,
    compact_motif_hit_buffer host_hits) {
    if (host_hits.records_written == nullptr || host_hits.records_dropped == nullptr
        || (host_hits.capacity > 0u && host_hits.hits == nullptr)
        || !valid_scan_input(sequence, motif)) {
        return baseplane::invalid_argument_status();
    }

    *host_hits.records_written = 0u;
    *host_hits.records_dropped = 0u;
    if (sequence.n_bases < motif.length) {
        return baseplane::ok_status();
    }

    const std::uint64_t windows = sequence.n_bases - motif.length + 1ULL;
    const dna2_word64 motif_word{motif.packed};
    for (std::uint64_t start = 0ULL; start < windows; ++start) {
        const dna2_word64 window{shifted_window_word64_host(sequence.words, sequence.n_words, start)};
        const int mismatches = detail::word64_mismatches_packed_count_fields(window, motif_word, motif.active_fields);
        if (mismatches > motif.max_mismatches) continue;

        const std::uint32_t slot = *host_hits.records_written;
        *host_hits.records_written = slot + 1u;
        if (slot < host_hits.capacity) {
            host_hits.hits[slot] = motif_hit{
                static_cast<std::uint32_t>(start),
                motif.motif_id,
                static_cast<std::uint8_t>(mismatches),
                0u
            };
        } else {
            *host_hits.records_dropped += 1u;
        }
    }

    return *host_hits.records_dropped == 0u
        ? baseplane::ok_status()
        : baseplane::capacity_exceeded_status(*host_hits.records_dropped);
}

void dna2_pack_ascii_batch_scalar(
    const char* input,
    std::size_t stride,
    dna2_word64* output,
    std::size_t count,
    std::size_t n_per_seq) {
    assert(input != nullptr || count == 0u || n_per_seq == 0u);
    assert(output != nullptr || count == 0u);
    for (std::size_t i = 0u; i < count; ++i) {
        output[i] = dna2_pack_ascii_32(input + i * stride, n_per_seq);
    }
}

void dna2_unpack_ascii_batch_scalar(
    const dna2_word64* input,
    char* output,
    std::size_t stride,
    std::size_t count,
    std::size_t n_per_seq) {
    assert(input != nullptr || count == 0u);
    assert(output != nullptr || count == 0u || n_per_seq == 0u);
    for (std::size_t i = 0u; i < count; ++i) {
        dna2_unpack_ascii_32(input[i], output + i * stride, n_per_seq);
    }
}

void dna2_to_planes_batch_scalar(const dna2_word64* input, dna2_planes32* output, std::size_t count) {
    assert(input != nullptr || count == 0u);
    assert(output != nullptr || count == 0u);
    for (std::size_t i = 0u; i < count; ++i) {
        output[i] = dna2_to_planes(input[i]);
    }
}

void planes_gc_mask_batch_scalar(const dna2_planes32* input, std::uint32_t* output_masks, std::size_t count) {
    assert(input != nullptr || count == 0u);
    assert(output_masks != nullptr || count == 0u);
    for (std::size_t i = 0u; i < count; ++i) {
        output_masks[i] = planes_gc_mask(input[i]);
    }
}

void planes_cpg_start_mask_batch_scalar(const dna2_planes32* input, std::uint32_t* output_masks, std::size_t count) {
    assert(input != nullptr || count == 0u);
    assert(output_masks != nullptr || count == 0u);
    for (std::size_t i = 0u; i < count; ++i) {
        output_masks[i] = planes_cpg_start_mask(input[i]);
    }
}

#if !BASEPLANE_ENABLE_HIGHWAY
void dna2_pack_ascii_batch_highway(
    const char* input,
    std::size_t stride,
    dna2_word64* output,
    std::size_t count,
    std::size_t n_per_seq) {
    dna2_pack_ascii_batch_scalar(input, stride, output, count, n_per_seq);
}

void dna2_unpack_ascii_batch_highway(
    const dna2_word64* input,
    char* output,
    std::size_t stride,
    std::size_t count,
    std::size_t n_per_seq) {
    dna2_unpack_ascii_batch_scalar(input, output, stride, count, n_per_seq);
}

void dna2_to_planes_batch_highway(const dna2_word64* input, dna2_planes32* output, std::size_t count) {
    dna2_to_planes_batch_scalar(input, output, count);
}

void planes_gc_mask_batch_highway(const dna2_planes32* input, std::uint32_t* output_masks, std::size_t count) {
    planes_gc_mask_batch_scalar(input, output_masks, count);
}

void planes_cpg_start_mask_batch_highway(const dna2_planes32* input, std::uint32_t* output_masks, std::size_t count) {
    planes_cpg_start_mask_batch_scalar(input, output_masks, count);
}
#endif

void dna2_pack_ascii_batch(
    const char* input,
    std::size_t stride,
    dna2_word64* output,
    std::size_t count,
    std::size_t n_per_seq) {
#if BASEPLANE_ENABLE_HIGHWAY
    dna2_pack_ascii_batch_highway(input, stride, output, count, n_per_seq);
#else
    dna2_pack_ascii_batch_scalar(input, stride, output, count, n_per_seq);
#endif
}

void dna2_unpack_ascii_batch(
    const dna2_word64* input,
    char* output,
    std::size_t stride,
    std::size_t count,
    std::size_t n_per_seq) {
#if BASEPLANE_ENABLE_HIGHWAY
    dna2_unpack_ascii_batch_highway(input, output, stride, count, n_per_seq);
#else
    dna2_unpack_ascii_batch_scalar(input, output, stride, count, n_per_seq);
#endif
}

void dna2_to_planes_batch(const dna2_word64* input, dna2_planes32* output, std::size_t count) {
#if BASEPLANE_ENABLE_HIGHWAY
    dna2_to_planes_batch_highway(input, output, count);
#else
    dna2_to_planes_batch_scalar(input, output, count);
#endif
}

void planes_gc_mask_batch(const dna2_planes32* input, std::uint32_t* output_masks, std::size_t count) {
#if BASEPLANE_ENABLE_HIGHWAY
    planes_gc_mask_batch_highway(input, output_masks, count);
#else
    planes_gc_mask_batch_scalar(input, output_masks, count);
#endif
}

void planes_cpg_start_mask_batch(const dna2_planes32* input, std::uint32_t* output_masks, std::size_t count) {
#if BASEPLANE_ENABLE_HIGHWAY
    planes_cpg_start_mask_batch_highway(input, output_masks, count);
#else
    planes_cpg_start_mask_batch_scalar(input, output_masks, count);
#endif
}

} // namespace baseplane::seq
