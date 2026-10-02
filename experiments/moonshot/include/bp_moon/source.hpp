#pragma once
#include <bp_moon/reference.hpp>

namespace bp_moon {
enum class Strand { forward, reverse };
struct SourceMap {
    u64 source_id, contig, origin, version;
    Strand strand;
    std::size_t length;
    u64 coordinate(std::size_t position) const {
        if (position >= length) throw std::out_of_range("source position");
        const auto offset = strand == Strand::forward ? position : length - 1 - position;
        if (offset > std::numeric_limits<u64>::max() - origin)
            throw std::overflow_error("source coordinate");
        return origin + offset;
    }
};
struct EmitCounts { u64 produced = 0, stored = 0, dropped = 0; };
// Selected invalid payloads never emit. Caller owns output and its capacity.
inline EmitCounts emit_valid_positions(const PackedFixture& sequence, u32 selected,
                                      std::size_t word, u64* output, std::size_t capacity,
                                      const SourceMap& source) {
    if (source.length != sequence.original.size()) throw std::invalid_argument("source length");
    if (capacity && !output) throw std::invalid_argument("null output");
    const u32 support = selected & sequence.planes(word).valid;
    EmitCounts counts;
    for (unsigned lane = 0; lane < 32; ++lane) {
        if (!(support & (u32{1} << lane))) continue;
        const auto position = word * 32 + lane;
        const auto coordinate = source.coordinate(position);
        ++counts.produced;
        if (counts.stored < capacity) output[counts.stored++] = coordinate;
        else ++counts.dropped;
    }
    return counts;
}
} // namespace bp_moon
