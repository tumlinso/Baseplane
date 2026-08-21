#include <Baseplane/seq/dna2_allowed.hh>

namespace baseplane::seq {
namespace {

bool valid_strand_mode(dna2_strand_mode mode) noexcept {
    return mode == dna2_strand_mode::forward
        || mode == dna2_strand_mode::reverse
        || mode == dna2_strand_mode::both;
}

bool valid_palindrome_policy(dna2_palindrome_policy policy) noexcept {
    return policy == dna2_palindrome_policy::deduplicate
        || policy == dna2_palindrome_policy::report_both;
}

std::uint32_t reverse_active_bits(std::uint32_t mask, std::uint8_t length) noexcept {
    std::uint32_t reversed = 0u;
    for (std::uint8_t position = 0u; position < length; ++position) {
        const std::uint32_t bit = (mask >> position) & 0x1u;
        reversed |= bit << static_cast<unsigned int>(length - 1u - position);
    }
    return reversed;
}

std::uint32_t allowed_mismatch_mask_unchecked(
    dna2_planes32 sequence,
    std::uint32_t validity_mask,
    const motif32_allowed& motif) noexcept {
    const std::uint32_t lo = sequence.lo;
    const std::uint32_t hi = sequence.hi;
    const std::uint32_t base_A = ~lo & ~hi;
    const std::uint32_t base_C = lo & ~hi;
    const std::uint32_t base_G = ~lo & hi;
    const std::uint32_t base_T = lo & hi;
    const std::uint32_t accepted = (base_A & motif.allow_A)
        | (base_C & motif.allow_C)
        | (base_G & motif.allow_G)
        | (base_T & motif.allow_T);
    return motif.active_mask & (~accepted | ~validity_mask);
}

} // namespace

bool dna2_make_motif32_allowed(
    const std::uint8_t* allowed_sets,
    std::uint8_t length,
    std::uint8_t max_mismatches,
    std::uint16_t motif_id,
    dna2_strand_mode strand_mode,
    dna2_palindrome_policy palindrome_policy,
    motif32_allowed* output) noexcept {
    if (output == nullptr || allowed_sets == nullptr || length == 0u || length > 32u) return false;
    std::uint32_t allow_A = 0u;
    std::uint32_t allow_C = 0u;
    std::uint32_t allow_G = 0u;
    std::uint32_t allow_T = 0u;
    for (std::uint8_t position = 0u; position < length; ++position) {
        const std::uint8_t set = allowed_sets[position];
        if (set == 0u || (set & 0xf0u) != 0u) return false;
        const std::uint32_t position_bit = 1u << position;
        if ((set & 0x1u) != 0u) allow_A |= position_bit;
        if ((set & 0x2u) != 0u) allow_C |= position_bit;
        if ((set & 0x4u) != 0u) allow_G |= position_bit;
        if ((set & 0x8u) != 0u) allow_T |= position_bit;
    }
    return dna2_normalize_motif32_allowed(
        allow_A,
        allow_C,
        allow_G,
        allow_T,
        length,
        max_mismatches,
        motif_id,
        strand_mode,
        palindrome_policy,
        output);
}

bool dna2_normalize_motif32_allowed(
    std::uint32_t allow_A,
    std::uint32_t allow_C,
    std::uint32_t allow_G,
    std::uint32_t allow_T,
    std::uint8_t length,
    std::uint8_t max_mismatches,
    std::uint16_t motif_id,
    dna2_strand_mode strand_mode,
    dna2_palindrome_policy palindrome_policy,
    motif32_allowed* output) noexcept {
    if (output == nullptr
        || length == 0u
        || length > 32u
        || max_mismatches > length
        || !valid_strand_mode(strand_mode)
        || !valid_palindrome_policy(palindrome_policy)) {
        return false;
    }
    const std::uint32_t active = detail::active_mask_from_length(length);
    motif32_allowed normalized{
        allow_A & active,
        allow_C & active,
        allow_G & active,
        allow_T & active,
        active,
        length,
        max_mismatches,
        motif_id,
        strand_mode,
        palindrome_policy
    };
    if ((normalized.allow_A | normalized.allow_C | normalized.allow_G | normalized.allow_T) != active) return false;
    *output = normalized;
    return true;
}

bool dna2_valid_motif32_allowed(const motif32_allowed& motif) noexcept {
    if (motif.length == 0u
        || motif.length > 32u
        || motif.max_mismatches > motif.length
        || !valid_strand_mode(motif.strand_mode)
        || !valid_palindrome_policy(motif.palindrome_policy)) {
        return false;
    }
    const std::uint32_t active = detail::active_mask_from_length(motif.length);
    if (motif.active_mask != active) return false;
    if (((motif.allow_A | motif.allow_C | motif.allow_G | motif.allow_T) & ~active) != 0u) return false;
    return (motif.allow_A | motif.allow_C | motif.allow_G | motif.allow_T) == active;
}

bool dna2_reverse_complement_allowed(
    const motif32_allowed& motif,
    motif32_allowed* output) noexcept {
    if (output == nullptr || !dna2_valid_motif32_allowed(motif)) return false;
    return dna2_normalize_motif32_allowed(
        reverse_active_bits(motif.allow_T, motif.length),
        reverse_active_bits(motif.allow_G, motif.length),
        reverse_active_bits(motif.allow_C, motif.length),
        reverse_active_bits(motif.allow_A, motif.length),
        motif.length,
        motif.max_mismatches,
        motif.motif_id,
        motif.strand_mode,
        motif.palindrome_policy,
        output);
}

bool dna2_allowed_is_palindrome(const motif32_allowed& motif) noexcept {
    motif32_allowed reverse{};
    if (!dna2_reverse_complement_allowed(motif, &reverse)) return false;
    return motif.allow_A == reverse.allow_A
        && motif.allow_C == reverse.allow_C
        && motif.allow_G == reverse.allow_G
        && motif.allow_T == reverse.allow_T;
}

bool dna2_allowed_mismatch_mask(
    dna2_planes32 sequence,
    std::uint32_t validity_mask,
    const motif32_allowed& motif,
    std::uint32_t* mismatch_mask) noexcept {
    if (mismatch_mask == nullptr || !dna2_valid_motif32_allowed(motif)) return false;
    *mismatch_mask = allowed_mismatch_mask_unchecked(sequence, validity_mask, motif);
    return true;
}

bool dna2_match_allowed_window(
    dna2_planes32 sequence,
    std::uint32_t validity_mask,
    const motif32_allowed& motif,
    dna2_allowed_match_result* result) noexcept {
    if (result == nullptr || !dna2_valid_motif32_allowed(motif)) return false;
    *result = dna2_allowed_match_result{0u, 0u, 0u, false};

    motif32_allowed reverse{};
    const bool evaluate_forward = motif.strand_mode != dna2_strand_mode::reverse;
    const bool evaluate_reverse = motif.strand_mode != dna2_strand_mode::forward;
    if (evaluate_reverse && !dna2_reverse_complement_allowed(motif, &reverse)) return false;

    if (evaluate_forward) {
        result->forward_mismatch_mask = allowed_mismatch_mask_unchecked(sequence, validity_mask, motif);
        if (detail::popcount32(result->forward_mismatch_mask) <= motif.max_mismatches) {
            result->strand_hits |= dna2_forward_strand_bit;
        }
    }
    if (evaluate_reverse) {
        result->reverse_mismatch_mask = allowed_mismatch_mask_unchecked(sequence, validity_mask, reverse);
        if (detail::popcount32(result->reverse_mismatch_mask) <= motif.max_mismatches) {
            result->strand_hits |= dna2_reverse_strand_bit;
        }
    }

    result->palindrome = dna2_allowed_is_palindrome(motif);
    if (motif.strand_mode == dna2_strand_mode::both
        && result->palindrome
        && motif.palindrome_policy == dna2_palindrome_policy::deduplicate
        && result->strand_hits != 0u) {
        result->strand_hits = dna2_forward_strand_bit;
    }
    return true;
}

bool dna2_normalize_motif32_exact(
    dna2_word64 motif_word,
    std::uint8_t length,
    std::uint8_t max_mismatches,
    std::uint16_t motif_id,
    motif32_exact* output) noexcept {
    if (output == nullptr || length == 0u || length > 32u || max_mismatches > length) return false;
    const std::uint32_t active = detail::active_mask_from_length(length);
    const std::uint64_t fields = detail::spread_active_mask_to_packed_fields(active);
    const std::uint64_t packed_mask = fields | (fields << 1u);
    *output = motif32_exact{motif_word.packed & packed_mask, fields, length, max_mismatches, motif_id};
    return true;
}

bool dna2_valid_motif32_exact(const motif32_exact& motif) noexcept {
    if (motif.length == 0u || motif.length > 32u || motif.max_mismatches > motif.length) return false;
    const std::uint32_t active = detail::active_mask_from_length(motif.length);
    const std::uint64_t fields = detail::spread_active_mask_to_packed_fields(active);
    const std::uint64_t packed_mask = fields | (fields << 1u);
    return motif.active_fields == fields && (motif.packed & ~packed_mask) == 0u;
}

} // namespace baseplane::seq
