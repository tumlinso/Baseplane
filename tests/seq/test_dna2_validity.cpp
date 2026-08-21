#include <Baseplane/seq/dna2_validity.hh>

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace seq = ::baseplane::seq;

namespace {

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

void test_encoding() {
    const char canonical[] = {'A', 'a', 'C', 'c', 'G', 'g', 'T', 't', 'U', 'u'};
    const std::uint8_t expected[] = {0u, 0u, 1u, 1u, 2u, 2u, 3u, 3u, 3u, 3u};
    for (std::size_t i = 0u; i < sizeof(canonical); ++i) {
        const seq::dna2_encoded_base encoded = seq::dna2_encode_base_with_validity(canonical[i]);
        require(encoded.valid && encoded.code == expected[i], "canonical base encoding mismatch");
    }
    for (char invalid : {'N', 'n', '-', '\0'}) {
        const seq::dna2_encoded_base encoded = seq::dna2_encode_base_with_validity(invalid);
        require(!encoded.valid, "noncanonical base became valid");
        require(encoded.code == 0u, "invalid placeholder code must remain bounded");
    }
}

void test_counts_and_tails() {
    struct count_case {
        std::uint64_t bases;
        std::uint64_t packed_words;
        std::uint64_t validity_words;
    };
    const count_case cases[] = {
        {0u, 0u, 0u}, {1u, 1u, 1u}, {31u, 1u, 1u}, {32u, 1u, 1u},
        {33u, 2u, 1u}, {63u, 2u, 1u}, {64u, 2u, 1u}, {65u, 3u, 2u}
    };
    for (const count_case& value : cases) {
        require(seq::dna2_packed64_word_count(value.bases) == value.packed_words, "packed word count mismatch");
        require(seq::dna2_stream32_word_count(value.bases) == value.packed_words, "stream word count mismatch");
        require(seq::dna2_validity64_word_count(value.bases) == value.validity_words, "validity word count mismatch");
    }

    require(seq::dna2_tail_mask32(0u, 0u) == 0u, "empty tail32 mask mismatch");
    require(seq::dna2_tail_mask32(32u, 0u) == 0xffffffffu, "full tail32 mask mismatch");
    require(seq::dna2_tail_mask32(33u, 1u) == 0x1u, "one-bit tail32 mask mismatch");
    require(seq::dna2_tail_mask32(65u, 2u) == 0x1u, "65-base tail32 mask mismatch");
    require(seq::dna2_tail_mask64(64u, 0u) == std::numeric_limits<std::uint64_t>::max(),
            "full tail64 mask mismatch");
    require(seq::dna2_tail_mask64(65u, 1u) == 0x1ULL, "one-bit tail64 mask mismatch");
    require(seq::dna2_tail_mask64(65u, 2u) == 0ULL, "out-of-range tail64 mask mismatch");
}

void test_packed_validity() {
    const std::uint64_t packed[3] = {0ULL, 0ULL, 0ULL};
    const std::uint64_t validity[2] = {
        std::numeric_limits<std::uint64_t>::max() ^ (1ULL << 32u),
        std::numeric_limits<std::uint64_t>::max()
    };
    const seq::dna2_packed64_valid_view view{
        seq::dna2_packed64_view{packed, 65u, 3u}, validity, 2u
    };
    require(seq::dna2_valid_view(view), "valid packed view rejected");
    require(seq::dna2_base_is_valid(view, 31u), "position 31 should be valid");
    require(!seq::dna2_base_is_valid(view, 32u), "position 32 should be invalid");
    require(seq::dna2_base_is_valid(view, 63u), "position 63 should be valid");
    require(seq::dna2_base_is_valid(view, 64u), "position 64 should be valid");
    require(!seq::dna2_base_is_valid(view, 65u), "tail position became valid");
    require(seq::dna2_validity_word(view, 1u) == 0x1ULL, "tail validity bits were not cleared");
    require(seq::dna2_window_is_valid(view, 0u, 32u), "valid prefix window rejected");
    require(!seq::dna2_window_is_valid(view, 31u, 2u), "invalid crossing window accepted");
    require(seq::dna2_window_is_valid(view, 33u, 32u), "valid cross-word window rejected");
    require(seq::dna2_window_is_valid(view, 65u, 0u), "empty end window rejected");
    require(!seq::dna2_window_is_valid(view, 65u, 1u), "out-of-range window accepted");

    const seq::dna2_packed64_valid_view implicit{
        seq::dna2_packed64_view{packed, 65u, 3u}, nullptr, 0u
    };
    require(seq::dna2_valid_view(implicit), "implicit-valid view rejected");
    require(seq::dna2_base_is_valid(implicit, 64u), "implicit-valid final base rejected");
    require(!seq::dna2_base_is_valid(implicit, 65u), "implicit validity leaked into tail");

    require(seq::dna2_valid_view(seq::dna2_packed64_valid_view{
                seq::dna2_packed64_view{nullptr, 0u, 0u}, nullptr, 0u}),
            "empty null view rejected");
    require(!seq::dna2_valid_view(seq::dna2_packed64_valid_view{
                seq::dna2_packed64_view{nullptr, 1u, 1u}, nullptr, 0u}),
            "nonempty null packed view accepted");
    require(!seq::dna2_valid_view(seq::dna2_packed64_valid_view{
                seq::dna2_packed64_view{packed, 65u, 2u}, validity, 2u}),
            "short packed view accepted");
    require(!seq::dna2_valid_view(seq::dna2_packed64_valid_view{
                seq::dna2_packed64_view{packed, 65u, 3u}, validity, 1u}),
            "short validity sidecar accepted");
}

void test_plane_and_mask_views() {
    const std::uint32_t lo[3] = {0u, 0u, 0u};
    const std::uint32_t hi[3] = {0u, 0u, 0u};
    const std::uint32_t masks[3] = {0xffffffffu, 0xffffffffu, 0xffffffffu};
    const std::uint32_t validity[3] = {0xffffffffu, 0xfffffffeu, 0xffffffffu};
    const seq::dna2_planes32_valid_stream_view planes{
        seq::dna2_planes32_stream_view{lo, hi, 3u}, validity, 65u
    };
    require(seq::dna2_valid_view(planes), "valid plane view rejected");
    require(seq::dna2_valid_mask(planes, 0u) == 0xffffffffu, "plane word 0 validity mismatch");
    require(seq::dna2_valid_mask(planes, 1u) == 0xfffffffeu, "plane word 1 validity mismatch");
    require(seq::dna2_valid_mask(planes, 2u) == 0x1u, "plane tail validity mismatch");

    const seq::dna2_mask32_valid_stream_view mask_view{
        seq::dna2_mask32_stream_view{masks, 3u}, validity, 65u
    };
    require(seq::dna2_valid_view(mask_view), "valid mask view rejected");
    require(seq::dna2_effective_mask_word(mask_view, 1u) == 0xfffffffeu,
            "mask validity was not applied");
    require(seq::dna2_effective_mask_word(mask_view, 2u) == 0x1u,
            "mask tail was not applied");
    require(seq::dna2_effective_mask_word(mask_view, 3u) == 0u,
            "out-of-range mask word was not cleared");

    require(seq::dna2_valid_view(seq::dna2_planes32_valid_stream_view{
                seq::dna2_planes32_stream_view{nullptr, nullptr, 0u}, nullptr, 0u}),
            "empty plane view rejected");
    require(!seq::dna2_valid_view(seq::dna2_planes32_valid_stream_view{
                seq::dna2_planes32_stream_view{nullptr, hi, 1u}, nullptr, 1u}),
            "nonempty null plane view accepted");
    require(!seq::dna2_valid_view(seq::dna2_mask32_valid_stream_view{
                seq::dna2_mask32_stream_view{nullptr, 1u}, nullptr, 1u}),
            "nonempty null mask view accepted");
}

void test_chunk_coordinates() {
    static_assert(std::is_trivially_copyable_v<seq::dna2_chunk_coordinates>);
    const seq::dna2_chunk_coordinates chunk{
        seq::dna2_chunk_identity{7u, 11u, 13u}, 1000u, 65u, 8u, 60u, 8u, 5u
    };
    require(seq::dna2_valid_chunk_coordinates(chunk), "valid chunk rejected");
    require(seq::dna2_window_fits_chunk(chunk, 0u, 65u), "full chunk window rejected");
    require(seq::dna2_window_fits_chunk(chunk, 65u, 0u), "empty end chunk window rejected");
    require(!seq::dna2_window_fits_chunk(chunk, 64u, 2u), "cross-boundary window accepted");
    require(!seq::dna2_anchor_is_owned(chunk, 7u), "left halo anchor marked owned");
    require(seq::dna2_anchor_is_owned(chunk, 8u), "owned-begin anchor rejected");
    require(seq::dna2_anchor_is_owned(chunk, 59u), "last owned anchor rejected");
    require(!seq::dna2_anchor_is_owned(chunk, 60u), "right halo anchor marked owned");

    for (std::uint32_t local : {0u, 31u, 32u, 63u, 64u}) {
        std::uint64_t global = 0u;
        require(seq::dna2_local_to_global(chunk, local, &global), "local-to-global conversion failed");
        require(global == 1000u + local, "local-to-global conversion mismatch");
    }
    std::uint64_t global = 0u;
    require(!seq::dna2_local_to_global(chunk, 65u, &global), "tail local position converted");
    require(!seq::dna2_local_to_global(chunk, 0u, nullptr), "null global output accepted");

    seq::dna2_chunk_coordinates invalid = chunk;
    invalid.base_count = static_cast<std::uint32_t>(seq::dna2_max_local_base_count + 1u);
    require(!seq::dna2_valid_chunk_coordinates(invalid), "oversized local chunk accepted");
    invalid = chunk;
    invalid.owned_begin = 9u;
    require(!seq::dna2_valid_chunk_coordinates(invalid), "inconsistent left halo accepted");
    invalid = chunk;
    invalid.global_base_begin = std::numeric_limits<std::uint64_t>::max() - 63u;
    require(!seq::dna2_valid_chunk_coordinates(invalid), "overflowing global range accepted");

    const seq::dna2_chunk_coordinates empty{
        seq::dna2_chunk_identity{0u, 0u, 0u}, std::numeric_limits<std::uint64_t>::max(), 0u, 0u, 0u, 0u, 0u
    };
    require(seq::dna2_valid_chunk_coordinates(empty), "empty chunk rejected");
    require(seq::dna2_window_fits_chunk(empty, 0u, 0u), "empty chunk window rejected");
}

} // namespace

int main() {
    test_encoding();
    test_counts_and_tails();
    test_packed_validity();
    test_plane_and_mask_views();
    test_chunk_coordinates();
    return 0;
}
