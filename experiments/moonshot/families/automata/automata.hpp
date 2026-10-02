#pragma once
#include <bp_moon/source.hpp>
#include <optional>
#include <string_view>

namespace bp_moon::automata {
inline char canonical(char base) {
    switch (base) {
    case 'a': return 'A'; case 'c': return 'C'; case 'g': return 'G'; case 't': return 'T';
    default: return base;
    }
}

// E05: every node carries the effect for every entry state; source bounds are local.
struct EffectNode {
    std::size_t begin, end;
    ce_moon::Dfa32 effect;
};
struct EffectHierarchy {
    std::vector<EffectNode> nodes;
    std::size_t root;
    explicit EffectHierarchy(const PackedFixture& source) : root(build(source.original, 0, source.original.size())) {}
private:
    std::size_t build(const std::string& source, std::size_t begin, std::size_t end) {
        if (end - begin <= 1) {
            nodes.push_back({begin, end, summarize_sequence(source.substr(begin, end - begin))});
        } else {
            const auto middle = begin + (end - begin) / 2;
            const auto left = build(source, begin, middle), right = build(source, middle, end);
            nodes.push_back({begin, end, ce_moon::compose(nodes[left].effect, nodes[right].effect)});
        }
        return nodes.size() - 1;
    }
};

// E06: toy motif AC. State 1 means previous valid base A; all other entries
// are no-prefix states. Witness offsets refer to the C endpoint, in sequence order.
struct WitnessedEffect {
    ce_moon::CountedDfa32 effect;
    std::array<std::optional<std::size_t>, 32> first{}, last{};
    std::size_t length = 0;
};
inline WitnessedEffect motif_base(char raw) {
    const auto base = canonical(raw);
    WitnessedEffect result;
    result.length = 1;
    for (unsigned state = 0; state < 32; ++state) {
        result.effect.state.to[state] = base == 'A' ? 1 : 0;
        result.effect.count[state] = state == 1 && base == 'C';
        if (result.effect.count[state]) result.first[state] = result.last[state] = 0;
    }
    return result;
}
inline WitnessedEffect concatenate(const WitnessedEffect& left, const WitnessedEffect& right) {
    if (right.length > std::numeric_limits<std::size_t>::max() - left.length)
        throw std::overflow_error("witness span");
    WitnessedEffect result;
    result.effect = ce_moon::compose(left.effect, right.effect);
    result.length = left.length + right.length;
    for (unsigned state = 0; state < 32; ++state) {
        const auto incoming = left.effect.state.to[state];
        const auto shifted = [&](std::optional<std::size_t> offset) -> std::optional<std::size_t> {
            if (!offset) return {};
            if (*offset >= right.length) throw std::invalid_argument("witness offset");
            return left.length + *offset;
        };
        result.first[state] = left.first[state] ? left.first[state] : shifted(right.first[incoming]);
        result.last[state] = right.last[incoming] ? shifted(right.last[incoming]) : left.last[state];
    }
    return result;
}
inline WitnessedEffect summarize_motif(std::string_view sequence) {
    WitnessedEffect result;
    for (char base : sequence) result = concatenate(result, motif_base(base));
    return result;
}
// Revisit preserves source identity externally through SourceMap. Output is
// caller-owned; produced/stored/dropped are distinct. Invalid symbols reset.
inline EmitCounts replay_motif(const PackedFixture& sequence, const SourceMap& source,
                              u64* endpoints, std::size_t capacity, unsigned entry = 0) {
    if (source.length != sequence.original.size() || entry >= 32 || (capacity && !endpoints))
        throw std::invalid_argument("motif replay input");
    EmitCounts result;
    bool prefix = entry == 1;
    for (std::size_t index = 0; index < source.length; ++index) {
        const auto base = canonical(sequence.original[index]);
        if (prefix && base == 'C') {
            result.produced = ce_moon::checked_add(result.produced, 1);
            if (result.stored < capacity) endpoints[result.stored++] = source.coordinate(index);
            else result.dropped = ce_moon::checked_add(result.dropped, 1);
        }
        prefix = base == 'A';
    }
    return result;
}

// E07: synthetic paired-base syntax A...T and C...G. Bounded boundary stacks
// preserve exact tile composition until capacity is exceeded; then replay is required.
struct GrammarTile {
    std::vector<char> unmatched_closes, unmatched_opens;
    std::int64_t net_depth = 0, minimum = 0, maximum = 0;
    bool mismatch = false, needs_replay = false;
};
inline bool matching(char open, char close) {
    return (open == 'A' && close == 'T') || (open == 'C' && close == 'G');
}
inline GrammarTile grammar_tile(std::string_view sequence, std::size_t capacity) {
    if (sequence.size() > static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max()))
        throw std::length_error("grammar span");
    GrammarTile result;
    for (char raw : sequence) {
        const auto base = canonical(raw);
        if (base == 'A' || base == 'C') {
            ++result.net_depth;
            if (result.unmatched_opens.size() < capacity) result.unmatched_opens.push_back(base);
            else result.needs_replay = true;
        } else if (base == 'T' || base == 'G') {
            --result.net_depth;
            if (!result.needs_replay) {
                if (!result.unmatched_opens.empty()) {
                    result.mismatch |= !matching(result.unmatched_opens.back(), base);
                    result.unmatched_opens.pop_back();
                } else if (result.unmatched_closes.size() < capacity) result.unmatched_closes.push_back(base);
                else result.needs_replay = true;
            }
        } else result.needs_replay = true;
        result.minimum = std::min(result.minimum, result.net_depth);
        result.maximum = std::max(result.maximum, result.net_depth);
        if (result.maximum > static_cast<std::int64_t>(std::min(capacity,
                static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max())))) result.needs_replay = true;
    }
    return result;
}
inline GrammarTile concatenate(const GrammarTile& left, const GrammarTile& right, std::size_t capacity) {
    // Small toy sequences are bounded explicitly before signed depth arithmetic.
    const auto limit = std::numeric_limits<std::int64_t>::max() / 4;
    if (left.net_depth < -limit || left.net_depth > limit || right.net_depth < -limit || right.net_depth > limit ||
        left.minimum < -limit || left.minimum > limit || right.minimum < -limit || right.minimum > limit ||
        left.maximum < -limit || left.maximum > limit || right.maximum < -limit || right.maximum > limit)
        throw std::overflow_error("grammar depth");
    GrammarTile result = left;
    result.net_depth = left.net_depth + right.net_depth;
    result.minimum = std::min(left.minimum, left.net_depth + right.minimum);
    result.maximum = std::max(left.maximum, left.net_depth + right.maximum);
    result.mismatch |= right.mismatch;
    result.needs_replay |= right.needs_replay;
    if (result.needs_replay) return result;
    for (char close : right.unmatched_closes) {
        if (!result.unmatched_opens.empty()) {
            result.mismatch |= !matching(result.unmatched_opens.back(), close);
            result.unmatched_opens.pop_back();
        } else result.unmatched_closes.push_back(close);
    }
    result.unmatched_opens.insert(result.unmatched_opens.end(), right.unmatched_opens.begin(), right.unmatched_opens.end());
    result.needs_replay = result.unmatched_opens.size() > capacity || result.unmatched_closes.size() > capacity ||
        result.maximum > static_cast<std::int64_t>(std::min(capacity,
                static_cast<std::size_t>(std::numeric_limits<std::int64_t>::max())));
    if (result.needs_replay) {
        result.unmatched_opens.resize(std::min(capacity, result.unmatched_opens.size()));
        result.unmatched_closes.resize(std::min(capacity, result.unmatched_closes.size()));
    }
    return result;
}
inline bool grammar_replay(std::string_view sequence) {
    std::vector<char> stack;
    for (char raw : sequence) {
        const auto base = canonical(raw);
        if (base == 'A' || base == 'C') stack.push_back(base);
        else if (base == 'T' || base == 'G') {
            if (stack.empty() || !matching(stack.back(), base)) return false;
            stack.pop_back();
        } else return false; // Unknown payload cannot establish syntax acceptance.
    }
    return stack.empty();
}
inline bool grammar_accepts(const GrammarTile& tile) {
    if (tile.needs_replay) throw std::logic_error("source replay required");
    return !tile.mismatch && tile.unmatched_opens.empty() && tile.unmatched_closes.empty();
}
} // namespace bp_moon::automata
