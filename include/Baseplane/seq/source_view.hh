#pragma once

#include <Baseplane/seq/dna2_validity.hh>

namespace baseplane::seq {

enum class source_strand : std::uint8_t { forward = 1, reverse = 2 };

// Tokens are declared by the source owner. Mutation requires a new generation;
// changes to coordinates/ownership require a new structure epoch.
struct source_stamp {
    dna2_chunk_identity identity;
    std::uint64_t structure_epoch;
    std::uint64_t value_generation;
};

inline bool operator==(const source_stamp& a, const source_stamp& b) noexcept {
    return a.identity.genome == b.identity.genome
        && a.identity.contig == b.identity.contig
        && a.identity.chunk == b.identity.chunk
        && a.structure_epoch == b.structure_epoch
        && a.value_generation == b.value_generation;
}
inline bool operator!=(const source_stamp& a, const source_stamp& b) noexcept {
    return !(a == b);
}

// Borrowed host view. Packed bases are stored in increasing genomic coordinates.
// Pointer lifetime and synchronization stay with the source owner.
struct source_view {
    dna2_packed64_valid_view sequence;
    dna2_chunk_coordinates coordinates;
    std::uint64_t structure_epoch;
    std::uint64_t value_generation;
};

inline source_stamp stamp(const source_view& source) noexcept {
    return {source.coordinates.identity, source.structure_epoch, source.value_generation};
}
inline bool valid_source(const source_view& source) noexcept {
    return dna2_valid_view(source.sequence)
        && dna2_valid_chunk_coordinates(source.coordinates)
        && source.sequence.packed.n_bases == source.coordinates.base_count;
}
inline bool valid_strand(source_strand strand) noexcept {
    return strand == source_strand::forward || strand == source_strand::reverse;
}

struct source_base {
    std::uint64_t global_position;
    std::uint32_t physical_position;
    std::uint8_t code;
    bool valid;
    bool owned;
};

// Logical reverse positions traverse reverse-complement sequence. The global
// coordinate names the original base, even when the returned code is complemented.
// Invalid payload is returned as code zero with valid=false, never decoded as A.
inline bool read_source_base(const source_view& source, source_strand strand,
                             std::uint32_t logical_position, source_base* output) noexcept {
    if (!output || !valid_source(source) || !valid_strand(strand)
        || logical_position >= source.coordinates.base_count) return false;
    const auto physical = strand == source_strand::forward ? logical_position
        : source.coordinates.base_count - 1u - logical_position;
    source_base result{};
    result.physical_position = physical;
    if (!dna2_local_to_global(source.coordinates, physical, &result.global_position)) return false;
    result.valid = dna2_base_is_valid(source.sequence, physical);
    result.owned = dna2_anchor_is_owned(source.coordinates, physical);
    if (result.valid) {
        result.code = static_cast<std::uint8_t>(
            (source.sequence.packed.words[physical / 32u] >> (2u * (physical % 32u))) & 3u);
        if (strand == source_strand::reverse) result.code ^= 3u;
    }
    *output = result;
    return true;
}

} // namespace baseplane::seq
