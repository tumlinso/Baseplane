#pragma once
#include <bp_moon/source.hpp>
#include <ce_moon/effects.hpp>
#include <string_view>

namespace bp_moon::response {
inline int base_code(char base) {
    switch (base) {
    case 'A': case 'a': return 0;
    case 'C': case 'c': return 1;
    case 'G': case 'g': return 2;
    case 'T': case 't': return 3;
    default: return -1;
    }
}
using State = std::array<double, 2>;
using Monomial = ce_moon::MonomialAffine<2>;
// E09 sequence vocabulary: C swaps channels; canonical bases scale/offset.
// Unknown symbols explicitly reset both channels, independent of packed payload.
inline Monomial monomial_base(char base) {
    const int code = base_code(base);
    auto effect = Monomial::identity();
    if (code < 0) { effect.d.fill(0); return effect; }
    if (code == 1) effect.p = {1, 0};
    effect.d = {1.0 + code * 0.125, 1.0 - code * 0.0625};
    effect.b = {double(code + 1), double(-code)};
    return effect;
}
inline Monomial summarize_monomial(std::string_view sequence) {
    auto effect = Monomial::identity();
    for (char base : sequence) effect = ce_moon::compose(effect, monomial_base(base));
    effect.validate();
    return effect;
}
struct HierarchyNode {
    std::size_t begin, end;
    Monomial effect;
};
struct ResponseHierarchy {
    SourceMap source;
    std::vector<HierarchyNode> nodes;
    std::size_t root;
    ResponseHierarchy(const PackedFixture& sequence, SourceMap map)
        : source(map), root(build_checked(sequence)) {}
private:
    std::size_t build_checked(const PackedFixture& sequence) {
        if (source.length != sequence.original.size()) throw std::invalid_argument("response source length");
        return build(sequence.original, 0, source.length);
    }
    std::size_t build(const std::string& sequence, std::size_t begin, std::size_t end) {
        if (end - begin <= 1) nodes.push_back({begin, end, summarize_monomial(std::string_view(sequence).substr(begin, end - begin))});
        else {
            const auto middle = begin + (end - begin) / 2;
            const auto left = build(sequence, begin, middle), right = build(sequence, middle, end);
            nodes.push_back({begin, end, ce_moon::compose(nodes[left].effect, nodes[right].effect)});
        }
        return nodes.size() - 1;
    }
};

// E10: two blocks of two channels, with source-driven block routing and dense
// mixing within each block. Composition remains exclusively the CE provider.
using Blocks = ce_moon::effects::BlockAffine<2, 2>;
using BlockState = std::array<double, 4>;
inline Blocks block_base(char base) {
    const int code = base_code(base);
    auto effect = Blocks::identity();
    if (code < 0) {
        for (auto& matrix : effect.matrix) for (auto& row : matrix) row.fill(0);
        return effect;
    }
    if (code == 1) effect.p = {1, 0};
    for (std::size_t block = 0; block < 2; ++block) {
        effect.matrix[block] = {{{1.0, 0.125 * (code + 1)}, {-0.0625 * code, 1.0}}};
        effect.bias[block] = {double(code + block), double(block) - code};
    }
    return effect;
}
inline Blocks summarize_blocks(std::string_view sequence) {
    auto result = Blocks::identity();
    for (char base : sequence) result = ce_moon::effects::compose(result, block_base(base));
    return result;
}

struct BlockRegion {
    SourceMap source;
    Blocks effect;
    BlockRegion(const PackedFixture& sequence, SourceMap map) : source(map), effect(summarize_blocks(sequence.original)) {
        if (source.length != sequence.original.size()) throw std::invalid_argument("block source length");
    }
};

// E11: each base contributes x -> x + alpha*x*x, a known toy nonlinear
// response. Jets cache its composed local approximation; exact source replay
// provides the omitted higher orders and establishes the expansion point.
inline double curvature(char base) {
    switch (base_code(base)) { case 0: return 0.1; case 1: return 0.2; case 2: return -0.05; default: return 0; }
}
inline double exact_response(std::string_view sequence, double input) {
    if (!std::isfinite(input)) throw std::invalid_argument("nonfinite response query");
    for (char base : sequence) {
        if (base_code(base) < 0) input = 0;
        else input = ce_moon::effects::finite(input + curvature(base) * input * input);
    }
    return input;
}
inline ce_moon::effects::Jet sequence_jet(std::string_view sequence, double center, double radius) {
    ce_moon::effects::Jet result{center, center, 1, 0, radius};
    ce_moon::effects::validate(result);
    for (char base : sequence) {
        const auto expansion = result.value;
        const auto alpha = curvature(base);
        const bool valid = base_code(base) >= 0;
        ce_moon::effects::Jet next{expansion,
            valid ? ce_moon::effects::finite(expansion + alpha * expansion * expansion) : 0,
            valid ? 1 + 2 * alpha * expansion : 0, valid ? alpha : 0, radius};
        result = ce_moon::effects::compose(result, next);
    }
    return result;
}
struct ResponseQuery {
    double value;
    bool refined;
    std::size_t source_bases_replayed;
    std::size_t source_bases_reanchored;
    bool certified_error_bound = false;
};
class JetRegion {
    PackedFixture sequence_;
    SourceMap source_;
    double radius_;
    ce_moon::effects::Jet jet_;
public:
    JetRegion(PackedFixture sequence, SourceMap source, double center, double radius)
        : sequence_(std::move(sequence)), source_(source), radius_(radius),
          jet_(sequence_jet(sequence_.original, center, radius)) {
        if (source.length != sequence_.original.size()) throw std::invalid_argument("jet source length");
    }
    const SourceMap& source() const { return source_; }
    const ce_moon::effects::Jet& jet() const { return jet_; }
    ResponseQuery query(double input) {
        if (!std::isfinite(input)) throw std::invalid_argument("nonfinite response query");
        if (std::abs(input - jet_.center) <= jet_.radius) {
            const auto candidate = ce_moon::effects::query(jet_, input);
            return {candidate.estimate, false, 0, 0};
        }
        const auto exact = exact_response(sequence_.original, input);
        jet_ = sequence_jet(sequence_.original, input, radius_);
        return {exact, true, sequence_.original.size(), sequence_.original.size()};
    }
};

// E12: forward prefix and reverse-order prefix are separate context recurrences.
// Right context does not invert a left effect or reverse-complement the source.
struct ContextQuery {
    State left, right;
    std::size_t left_replayed, right_replayed;
};
class SequenceCheckpoints {
    SourceMap source_;
    ce_moon::effects::Checkpoints<2> forward_, backward_;
    static std::vector<Monomial> effects(std::string_view sequence, bool reverse) {
        std::vector<Monomial> result;
        result.reserve(sequence.size());
        if (reverse) for (std::size_t i = sequence.size(); i > 0; --i) result.push_back(monomial_base(sequence[i - 1]));
        else for (char base : sequence) result.push_back(monomial_base(base));
        return result;
    }
public:
    SequenceCheckpoints(const PackedFixture& sequence, SourceMap source, std::size_t stride)
        : source_(source), forward_(effects(sequence.original, false), stride),
          backward_(effects(sequence.original, true), stride) {
        if (source_.length != sequence.original.size()) throw std::invalid_argument("checkpoint source length");
    }
    ContextQuery query(std::size_t boundary, const State& left_input, const State& right_input) const {
        if (boundary > source_.length) throw std::out_of_range("sequence boundary");
        ContextQuery result;
        result.left = forward_.from_left(boundary, left_input, &result.left_replayed);
        result.right = backward_.from_left(source_.length - boundary, right_input, &result.right_replayed);
        return result;
    }
    State forward_suffix(std::size_t boundary, const State& input, std::size_t* replayed = nullptr) const {
        return forward_.from_right(boundary, input, replayed);
    }
    const SourceMap& source() const { return source_; }
    std::size_t retained_maps() const { return forward_.retained_maps() + backward_.retained_maps(); }
};
} // namespace bp_moon::response
