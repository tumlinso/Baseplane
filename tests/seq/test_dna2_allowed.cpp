#include <Baseplane/seq/dna2_allowed.hh>

#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

namespace seq = ::baseplane::seq;

namespace {

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

seq::dna2_planes32 planes_from_codes(const std::vector<std::uint8_t>& codes) {
    seq::dna2_planes32 planes{0u, 0u};
    for (std::size_t position = 0u; position < codes.size(); ++position) {
        planes.lo |= static_cast<std::uint32_t>(codes[position] & 0x1u) << position;
        planes.hi |= static_cast<std::uint32_t>((codes[position] >> 1u) & 0x1u) << position;
    }
    return planes;
}

std::uint32_t active_mask(std::uint8_t length) {
    return length == 32u ? 0xffffffffu : ((1u << length) - 1u);
}

std::uint8_t complement_set(std::uint8_t set) {
    return static_cast<std::uint8_t>(
        ((set & 0x1u) << 3u)
        | ((set & 0x2u) << 1u)
        | ((set & 0x4u) >> 1u)
        | ((set & 0x8u) >> 3u));
}

std::vector<std::uint8_t> reverse_complement_sets(const std::vector<std::uint8_t>& sets) {
    std::vector<std::uint8_t> reverse(sets.size(), 0u);
    for (std::size_t position = 0u; position < sets.size(); ++position) {
        reverse[sets.size() - 1u - position] = complement_set(sets[position]);
    }
    return reverse;
}

std::uint32_t mismatch_ref(
    const std::vector<std::uint8_t>& codes,
    const std::vector<std::uint8_t>& sets,
    std::uint32_t validity) {
    std::uint32_t mismatches = 0u;
    for (std::size_t position = 0u; position < sets.size(); ++position) {
        const bool valid = ((validity >> position) & 0x1u) != 0u;
        const bool allowed = (sets[position] & (1u << codes[position])) != 0u;
        if (!valid || !allowed) mismatches |= 1u << position;
    }
    return mismatches;
}

int popcount(std::uint32_t value) {
    int count = 0;
    while (value != 0u) {
        count += static_cast<int>(value & 0x1u);
        value >>= 1u;
    }
    return count;
}

seq::motif32_allowed make_allowed(
    const std::vector<std::uint8_t>& sets,
    std::uint8_t max_mismatches,
    seq::dna2_strand_mode strand_mode = seq::dna2_strand_mode::forward,
    seq::dna2_palindrome_policy palindrome_policy = seq::dna2_palindrome_policy::deduplicate) {
    seq::motif32_allowed motif{};
    require(seq::dna2_make_motif32_allowed(
                sets.data(),
                static_cast<std::uint8_t>(sets.size()),
                max_mismatches,
                19u,
                strand_mode,
                palindrome_policy,
                &motif),
            "allowed motif normalization failed");
    return motif;
}

void test_all_subsets() {
    for (std::uint8_t set = 1u; set <= 15u; ++set) {
        const seq::motif32_allowed motif = make_allowed({set}, 0u);
        require(seq::dna2_valid_motif32_allowed(motif), "normalized subset motif rejected");
        for (std::uint8_t code = 0u; code < 4u; ++code) {
            seq::dna2_allowed_match_result result{};
            require(seq::dna2_match_allowed_window(
                        planes_from_codes({code}), 0x1u, motif, &result),
                    "subset match evaluation failed");
            const bool expected = (set & (1u << code)) != 0u;
            require(((result.strand_hits & seq::dna2_forward_strand_bit) != 0u) == expected,
                    "allowed subset membership mismatch");
        }
    }
}

void test_invalid_inputs_and_exact_normalization() {
    seq::motif32_allowed allowed{};
    const std::uint8_t zero_set[] = {0u};
    const std::uint8_t high_set[] = {0x10u};
    const std::uint8_t one_set[] = {0x1u};
    require(!seq::dna2_make_motif32_allowed(
                zero_set, 1u, 0u, 0u, seq::dna2_strand_mode::forward,
                seq::dna2_palindrome_policy::deduplicate, &allowed),
            "empty allowed set accepted");
    require(!seq::dna2_make_motif32_allowed(
                high_set, 1u, 0u, 0u, seq::dna2_strand_mode::forward,
                seq::dna2_palindrome_policy::deduplicate, &allowed),
            "out-of-domain allowed bits accepted");
    require(!seq::dna2_make_motif32_allowed(
                nullptr, 1u, 0u, 0u, seq::dna2_strand_mode::forward,
                seq::dna2_palindrome_policy::deduplicate, &allowed),
            "null allowed sets accepted");
    require(!seq::dna2_make_motif32_allowed(
                one_set, 0u, 0u, 0u, seq::dna2_strand_mode::forward,
                seq::dna2_palindrome_policy::deduplicate, &allowed),
            "zero-length allowed motif accepted");
    require(!seq::dna2_make_motif32_allowed(
                one_set, 1u, 2u, 0u, seq::dna2_strand_mode::forward,
                seq::dna2_palindrome_policy::deduplicate, &allowed),
            "oversized mismatch threshold accepted");

    allowed = make_allowed({0x1u, 0x2u}, 0u);
    allowed.active_mask = 0x1u;
    require(!seq::dna2_valid_motif32_allowed(allowed), "forged active mask accepted");
    allowed = make_allowed({0x1u, 0x2u}, 0u);
    allowed.allow_A |= 0x4u;
    require(!seq::dna2_valid_motif32_allowed(allowed), "inactive allowed bit accepted");
    allowed = make_allowed({0x1u, 0x2u}, 0u);
    allowed.allow_C = 0u;
    require(!seq::dna2_valid_motif32_allowed(allowed), "empty active position accepted");

    seq::motif32_exact exact{};
    require(seq::dna2_normalize_motif32_exact(
                seq::dna2_word64{0xffffffffffffffffULL}, 1u, 0u, 23u, &exact),
            "exact normalization failed");
    require(seq::dna2_valid_motif32_exact(exact), "normalized exact motif rejected");
    require(exact.packed == 0x3u, "exact normalization did not clear inactive fields");
    exact.active_fields = 0u;
    require(!seq::dna2_valid_motif32_exact(exact), "forged exact derived field accepted");
    require(!seq::dna2_normalize_motif32_exact(
                seq::dna2_word64{0u}, 0u, 0u, 0u, &exact),
            "zero-length exact motif accepted");
    require(!seq::dna2_normalize_motif32_exact(
                seq::dna2_word64{0u}, 1u, 2u, 0u, &exact),
            "oversized exact mismatch threshold accepted");
}

void test_random_oracle() {
    std::mt19937 rng(20260821u);
    for (std::uint8_t length : {1u, 8u, 16u, 31u, 32u}) {
        for (int trial = 0; trial < 200; ++trial) {
            std::vector<std::uint8_t> sets(length, 0u);
            std::vector<std::uint8_t> codes(length, 0u);
            for (std::uint8_t position = 0u; position < length; ++position) {
                sets[position] = static_cast<std::uint8_t>(1u + (rng() % 15u));
                codes[position] = static_cast<std::uint8_t>(rng() % 4u);
            }
            const std::uint32_t validity = rng() & active_mask(length);
            const std::uint8_t threshold = static_cast<std::uint8_t>(trial % (length + 1u));
            const std::vector<std::uint8_t> reverse_sets = reverse_complement_sets(sets);
            const std::uint32_t forward_ref = mismatch_ref(codes, sets, validity);
            const std::uint32_t reverse_ref = mismatch_ref(codes, reverse_sets, validity);
            const seq::motif32_allowed motif = make_allowed(
                sets,
                threshold,
                seq::dna2_strand_mode::both,
                seq::dna2_palindrome_policy::report_both);
            seq::dna2_allowed_match_result result{};
            require(seq::dna2_match_allowed_window(
                        planes_from_codes(codes), validity, motif, &result),
                    "random allowed evaluation failed");
            require(result.forward_mismatch_mask == forward_ref, "random forward mismatch mask mismatch");
            require(result.reverse_mismatch_mask == reverse_ref, "random reverse mismatch mask mismatch");
            const std::uint8_t expected_hits = static_cast<std::uint8_t>(
                (popcount(forward_ref) <= threshold ? seq::dna2_forward_strand_bit : 0u)
                | (popcount(reverse_ref) <= threshold ? seq::dna2_reverse_strand_bit : 0u));
            require(result.strand_hits == expected_hits, "random threshold/strand mismatch");
        }
    }
}

void test_strands_palindromes_and_validity() {
    const std::vector<std::uint8_t> acg = {0x1u, 0x2u, 0x4u};
    const seq::dna2_planes32 forward_sequence = planes_from_codes({0u, 1u, 2u});
    const seq::dna2_planes32 reverse_sequence = planes_from_codes({1u, 2u, 3u});

    seq::dna2_allowed_match_result result{};
    require(seq::dna2_match_allowed_window(
                forward_sequence,
                0x7u,
                make_allowed(acg, 0u, seq::dna2_strand_mode::forward),
                &result),
            "forward-only evaluation failed");
    require(result.strand_hits == seq::dna2_forward_strand_bit, "forward-only hit mismatch");
    require(seq::dna2_match_allowed_window(
                reverse_sequence,
                0x7u,
                make_allowed(acg, 0u, seq::dna2_strand_mode::reverse),
                &result),
            "reverse-only evaluation failed");
    require(result.strand_hits == seq::dna2_reverse_strand_bit, "reverse-only hit mismatch");

    const std::vector<std::uint8_t> acgt = {0x1u, 0x2u, 0x4u, 0x8u};
    const seq::dna2_planes32 palindrome_sequence = planes_from_codes({0u, 1u, 2u, 3u});
    const seq::motif32_allowed deduplicated = make_allowed(
        acgt, 0u, seq::dna2_strand_mode::both, seq::dna2_palindrome_policy::deduplicate);
    require(seq::dna2_allowed_is_palindrome(deduplicated), "palindrome was not detected");
    require(seq::dna2_match_allowed_window(palindrome_sequence, 0xfu, deduplicated, &result),
            "deduplicated palindrome evaluation failed");
    require(result.palindrome && result.strand_hits == seq::dna2_forward_strand_bit,
            "palindrome duplicate was not removed");

    const seq::motif32_allowed report_both = make_allowed(
        acgt, 0u, seq::dna2_strand_mode::both, seq::dna2_palindrome_policy::report_both);
    require(seq::dna2_match_allowed_window(palindrome_sequence, 0xfu, report_both, &result),
            "report-both palindrome evaluation failed");
    require(result.strand_hits == (seq::dna2_forward_strand_bit | seq::dna2_reverse_strand_bit),
            "palindrome strands were not both reported");

    const seq::motif32_allowed any4 = make_allowed({0xfu, 0xfu, 0xfu, 0xfu}, 0u);
    require(seq::dna2_match_allowed_window(palindrome_sequence, 0xdu, any4, &result),
            "invalid-base evaluation failed");
    require(result.forward_mismatch_mask == 0x2u && result.strand_hits == 0u,
            "invalid base did not force a mismatch");
    const seq::motif32_allowed any4_one_mismatch = make_allowed({0xfu, 0xfu, 0xfu, 0xfu}, 1u);
    require(seq::dna2_match_allowed_window(palindrome_sequence, 0xdu, any4_one_mismatch, &result),
            "invalid-base threshold evaluation failed");
    require(result.strand_hits == seq::dna2_forward_strand_bit,
            "invalid base was not counted against the threshold exactly once");
}

} // namespace

int main() {
    test_all_subsets();
    test_invalid_inputs_and_exact_normalization();
    test_random_oracle();
    test_strands_palindromes_and_validity();
    return 0;
}
