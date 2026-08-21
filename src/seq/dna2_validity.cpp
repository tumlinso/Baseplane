#include <Baseplane/seq/dna2_validity.hh>

#include <limits>

namespace baseplane::seq {
namespace {

std::uint64_t low_mask64(std::uint64_t bits) noexcept {
    if (bits == 0u) return 0ULL;
    if (bits >= 64u) return std::numeric_limits<std::uint64_t>::max();
    return (1ULL << bits) - 1ULL;
}

std::uint32_t low_mask32(std::uint64_t bits) noexcept {
    if (bits == 0u) return 0u;
    if (bits >= 32u) return std::numeric_limits<std::uint32_t>::max();
    return (1u << static_cast<unsigned int>(bits)) - 1u;
}

} // namespace

std::uint64_t dna2_packed64_word_count(std::uint64_t base_count) noexcept {
    return (base_count / 32u) + static_cast<std::uint64_t>((base_count % 32u) != 0u);
}

std::uint64_t dna2_validity64_word_count(std::uint64_t base_count) noexcept {
    return (base_count / 64u) + static_cast<std::uint64_t>((base_count % 64u) != 0u);
}

std::uint64_t dna2_stream32_word_count(std::uint64_t base_count) noexcept {
    return dna2_packed64_word_count(base_count);
}

std::uint64_t dna2_tail_mask64(std::uint64_t base_count, std::uint64_t word_index) noexcept {
    const std::uint64_t words = dna2_validity64_word_count(base_count);
    if (word_index >= words) return 0ULL;
    const std::uint64_t remaining = base_count - (word_index * 64u);
    return low_mask64(remaining);
}

std::uint32_t dna2_tail_mask32(std::uint64_t base_count, std::uint64_t word_index) noexcept {
    const std::uint64_t words = dna2_stream32_word_count(base_count);
    if (word_index >= words) return 0u;
    const std::uint64_t remaining = base_count - (word_index * 32u);
    return low_mask32(remaining);
}

dna2_encoded_base dna2_encode_base_with_validity(char base) noexcept {
    switch (base) {
        case 'A': case 'a': return dna2_encoded_base{0u, true};
        case 'C': case 'c': return dna2_encoded_base{1u, true};
        case 'G': case 'g': return dna2_encoded_base{2u, true};
        case 'T': case 't': case 'U': case 'u': return dna2_encoded_base{3u, true};
        default: return dna2_encoded_base{0u, false};
    }
}

bool dna2_valid_view(const dna2_packed64_valid_view& view) noexcept {
    const std::uint64_t packed_words = dna2_packed64_word_count(view.packed.n_bases);
    if (view.packed.n_bases > 0u && view.packed.words == nullptr) return false;
    if (view.packed.n_words < packed_words) return false;
    if (view.validity_words == nullptr) return view.n_validity_words == 0u;
    return view.n_validity_words >= dna2_validity64_word_count(view.packed.n_bases);
}

std::uint64_t dna2_validity_word(
    const dna2_packed64_valid_view& view,
    std::uint64_t word_index) noexcept {
    if (!dna2_valid_view(view)) return 0ULL;
    const std::uint64_t tail = dna2_tail_mask64(view.packed.n_bases, word_index);
    if (tail == 0u) return 0ULL;
    return view.validity_words == nullptr ? tail : (view.validity_words[word_index] & tail);
}

bool dna2_base_is_valid(
    const dna2_packed64_valid_view& view,
    std::uint64_t local_position) noexcept {
    if (local_position >= view.packed.n_bases) return false;
    const std::uint64_t word = local_position / 64u;
    const std::uint64_t bit = local_position % 64u;
    return ((dna2_validity_word(view, word) >> bit) & 1ULL) != 0ULL;
}

bool dna2_window_is_valid(
    const dna2_packed64_valid_view& view,
    std::uint64_t local_begin,
    std::uint64_t length) noexcept {
    if (!dna2_valid_view(view) || local_begin > view.packed.n_bases) return false;
    if (length == 0u) return true;
    if (length > view.packed.n_bases - local_begin) return false;

    std::uint64_t cursor = local_begin;
    std::uint64_t remaining = length;
    while (remaining != 0u) {
        const std::uint64_t word = cursor / 64u;
        const std::uint64_t offset = cursor % 64u;
        const std::uint64_t available = 64u - offset;
        const std::uint64_t take = remaining < available ? remaining : available;
        const std::uint64_t required = low_mask64(take) << offset;
        if ((dna2_validity_word(view, word) & required) != required) return false;
        cursor += take;
        remaining -= take;
    }
    return true;
}

bool dna2_valid_view(const dna2_planes32_valid_stream_view& view) noexcept {
    const std::uint64_t words = dna2_stream32_word_count(view.base_count);
    if (view.base_count > 0u && (view.planes.lo_words == nullptr || view.planes.hi_words == nullptr)) return false;
    return view.planes.n_words >= words;
}

std::uint32_t dna2_valid_mask(
    const dna2_planes32_valid_stream_view& view,
    std::uint64_t word_index) noexcept {
    if (!dna2_valid_view(view)) return 0u;
    const std::uint32_t tail = dna2_tail_mask32(view.base_count, word_index);
    if (tail == 0u) return 0u;
    return view.validity_masks == nullptr ? tail : (view.validity_masks[word_index] & tail);
}

bool dna2_valid_view(const dna2_mask32_valid_stream_view& view) noexcept {
    const std::uint64_t words = dna2_stream32_word_count(view.base_count);
    if (view.base_count > 0u && view.masks.masks == nullptr) return false;
    return view.masks.n_words >= words;
}

std::uint32_t dna2_valid_mask(
    const dna2_mask32_valid_stream_view& view,
    std::uint64_t word_index) noexcept {
    if (!dna2_valid_view(view)) return 0u;
    const std::uint32_t tail = dna2_tail_mask32(view.base_count, word_index);
    if (tail == 0u) return 0u;
    return view.validity_masks == nullptr ? tail : (view.validity_masks[word_index] & tail);
}

std::uint32_t dna2_effective_mask_word(
    const dna2_mask32_valid_stream_view& view,
    std::uint64_t word_index) noexcept {
    if (!dna2_valid_view(view) || word_index >= view.masks.n_words) return 0u;
    return view.masks.masks[word_index] & dna2_valid_mask(view, word_index);
}

bool dna2_valid_chunk_coordinates(const dna2_chunk_coordinates& chunk) noexcept {
    if (static_cast<std::uint64_t>(chunk.base_count) > dna2_max_local_base_count) return false;
    if (chunk.owned_begin > chunk.owned_end || chunk.owned_end > chunk.base_count) return false;
    if (chunk.halo_left != chunk.owned_begin) return false;
    if (chunk.halo_right != chunk.base_count - chunk.owned_end) return false;
    if (chunk.base_count == 0u) return true;
    return chunk.global_base_begin
        <= std::numeric_limits<std::uint64_t>::max() - static_cast<std::uint64_t>(chunk.base_count - 1u);
}

bool dna2_window_fits_chunk(
    const dna2_chunk_coordinates& chunk,
    std::uint32_t local_begin,
    std::uint32_t length) noexcept {
    if (!dna2_valid_chunk_coordinates(chunk) || local_begin > chunk.base_count) return false;
    return length <= chunk.base_count - local_begin;
}

bool dna2_anchor_is_owned(
    const dna2_chunk_coordinates& chunk,
    std::uint32_t local_position) noexcept {
    return dna2_valid_chunk_coordinates(chunk)
        && local_position >= chunk.owned_begin
        && local_position < chunk.owned_end;
}

bool dna2_local_to_global(
    const dna2_chunk_coordinates& chunk,
    std::uint32_t local_position,
    std::uint64_t* global_position) noexcept {
    if (global_position == nullptr
        || !dna2_valid_chunk_coordinates(chunk)
        || local_position >= chunk.base_count) {
        return false;
    }
    *global_position = chunk.global_base_begin + static_cast<std::uint64_t>(local_position);
    return true;
}

} // namespace baseplane::seq
