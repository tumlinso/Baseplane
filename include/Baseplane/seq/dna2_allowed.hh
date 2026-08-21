#pragma once

#include <Baseplane/seq/dna2_motif.hh>

#include <cstdint>

namespace baseplane::seq {

enum class dna2_strand_mode : std::uint8_t {
    forward = 0,
    reverse = 1,
    both = 2,
};

enum class dna2_palindrome_policy : std::uint8_t {
    deduplicate = 0,
    report_both = 1,
};

inline constexpr std::uint8_t dna2_forward_strand_bit = 0x1u;
inline constexpr std::uint8_t dna2_reverse_strand_bit = 0x2u;

struct motif32_allowed {
    std::uint32_t allow_A;
    std::uint32_t allow_C;
    std::uint32_t allow_G;
    std::uint32_t allow_T;
    std::uint32_t active_mask;
    std::uint8_t length;
    std::uint8_t max_mismatches;
    std::uint16_t motif_id;
    dna2_strand_mode strand_mode;
    dna2_palindrome_policy palindrome_policy;
};

struct dna2_allowed_match_result {
    std::uint32_t forward_mismatch_mask;
    std::uint32_t reverse_mismatch_mask;
    std::uint8_t strand_hits;
    bool palindrome;
};

// Each allowed_sets element is a nonempty four-bit A/C/G/T subset. This is a
// direct normalized representation, not an IUPAC or motif-database parser.
bool dna2_make_motif32_allowed(
    const std::uint8_t* allowed_sets,
    std::uint8_t length,
    std::uint8_t max_mismatches,
    std::uint16_t motif_id,
    dna2_strand_mode strand_mode,
    dna2_palindrome_policy palindrome_policy,
    motif32_allowed* output) noexcept;

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
    motif32_allowed* output) noexcept;

bool dna2_valid_motif32_allowed(const motif32_allowed& motif) noexcept;
bool dna2_reverse_complement_allowed(
    const motif32_allowed& motif,
    motif32_allowed* output) noexcept;
bool dna2_allowed_is_palindrome(const motif32_allowed& motif) noexcept;

bool dna2_allowed_mismatch_mask(
    dna2_planes32 sequence,
    std::uint32_t validity_mask,
    const motif32_allowed& motif,
    std::uint32_t* mismatch_mask) noexcept;

bool dna2_match_allowed_window(
    dna2_planes32 sequence,
    std::uint32_t validity_mask,
    const motif32_allowed& motif,
    dna2_allowed_match_result* result) noexcept;

// Existing exact layout remains compatible. These helpers provide a validated,
// canonical construction route and reject forged derived fields.
bool dna2_normalize_motif32_exact(
    dna2_word64 motif_word,
    std::uint8_t length,
    std::uint8_t max_mismatches,
    std::uint16_t motif_id,
    motif32_exact* output) noexcept;
bool dna2_valid_motif32_exact(const motif32_exact& motif) noexcept;

} // namespace baseplane::seq
