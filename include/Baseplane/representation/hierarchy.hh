#pragma once

#include <Baseplane/seq/dna2_ops.hh>
#include <Baseplane/seq/dna2_validity.hh>
#include <Baseplane/seq/sequence_event.hh>
#include <ce_moon/reference.hpp>
#include <algorithm>
#include <functional>
#include <memory>

namespace baseplane::representation {

struct span { std::uint32_t begin, end; };
enum class strand : std::uint8_t { forward, reverse };
struct source_key {
    seq::dna2_chunk_identity identity;
    std::uint64_t structure_epoch, value_generation;
};
inline bool operator==(const source_key& a, const source_key& b) noexcept {
    return a.identity.genome == b.identity.genome && a.identity.contig == b.identity.contig
        && a.identity.chunk == b.identity.chunk && a.structure_epoch == b.structure_epoch
        && a.value_generation == b.value_generation;
}

// All representations and revisits share this immutable snapshot. Input is
// borrowed only during construction; no source pointer survives that copy.
class exact_source {
    seq::dna2_chunk_coordinates coordinates_;
    source_key key_;
    strand strand_;
    std::vector<std::uint64_t> packed_, validity_;
public:
    exact_source(seq::dna2_packed64_valid_view input, seq::dna2_chunk_coordinates coordinates,
                 std::uint64_t structure_epoch, std::uint64_t value_generation,
                 strand orientation = strand::forward)
        : coordinates_(coordinates), key_{coordinates.identity, structure_epoch, value_generation},
          strand_(orientation) {
        if (!seq::dna2_valid_view(input) || !seq::dna2_valid_chunk_coordinates(coordinates)
            || input.packed.n_bases != coordinates.base_count
            || (orientation != strand::forward && orientation != strand::reverse))
            throw std::invalid_argument("hierarchy source descriptor");
        const auto words = seq::dna2_packed64_word_count(input.packed.n_bases);
        if (words) packed_.assign(input.packed.words, input.packed.words + words);
        const auto valid_words = seq::dna2_validity64_word_count(input.packed.n_bases);
        for (std::uint64_t i = 0; i < valid_words; ++i)
            validity_.push_back(seq::dna2_validity_word(input, i));
    }
    std::uint32_t size() const noexcept { return coordinates_.base_count; }
    source_key key() const noexcept { return key_; }
    strand orientation() const noexcept { return strand_; }
    void check(span region) const {
        if (region.begin > region.end || region.end > size()) throw std::out_of_range("hierarchy span");
    }
    std::uint32_t physical(std::uint32_t logical) const {
        if (logical >= size()) throw std::out_of_range("source base");
        return strand_ == strand::forward ? logical : size() - 1 - logical;
    }
    int base(std::uint32_t logical) const {
        const auto p = physical(logical);
        if (!(validity_[p / 64] & (std::uint64_t{1} << (p % 64)))) return -1;
        const auto code = seq::get_base(seq::dna2_word64{packed_[p / 32]}, static_cast<int>(p % 32));
        return strand_ == strand::forward ? code : code ^ 3;
    }
    std::uint64_t coordinate(std::uint32_t logical) const {
        std::uint64_t result = 0;
        if (!seq::dna2_local_to_global(coordinates_, physical(logical), &result))
            throw std::out_of_range("source coordinate");
        return result;
    }
    bool owned(std::uint32_t logical) const {
        return seq::dna2_anchor_is_owned(coordinates_, physical(logical));
    }
};
using source_owner = std::shared_ptr<const exact_source>;

// Chosen or learned boundaries enter through the same callback. No preparatory
// hierarchy is assumed. The caller supplies fitted parameters/objective.
inline std::vector<std::uint32_t> choose_cuts(const exact_source& source,
    std::uint32_t minimum, std::uint32_t maximum, double threshold,
    const std::function<double(int, int, std::uint32_t)>& score) {
    if (!minimum || maximum < minimum || !std::isfinite(threshold) || !score)
        throw std::invalid_argument("chunk scorer");
    std::vector<std::uint32_t> cuts;
    std::uint32_t start = 0;
    for (std::uint32_t p = 1; p < source.size(); ++p) {
        const auto value = score(source.base(p - 1), source.base(p), p);
        if (!std::isfinite(value)) throw std::invalid_argument("nonfinite boundary score");
        if (p - start >= maximum || (p - start >= minimum && value > threshold)) {
            cuts.push_back(p); start = p;
        }
    }
    return cuts;
}

using effect = ce_moon::MonomialAffine<2>;
using state = std::array<double, 2>;
// Sequence-specific effects are caller vocabulary; generic composition is CE.
// Entry 4 supplies the caller's explicit invalid-base transition.
using vocabulary = std::array<effect, 5>;
struct summary {
    std::array<std::uint64_t, 4> counts{};
    std::uint64_t invalid = 0;
    effect ordered = effect::identity();
};
struct query_result { summary value; std::size_t summary_nodes = 0, replayed_bases = 0; };
struct detail { std::uint64_t coordinate; std::uint32_t logical; std::uint8_t code; bool owned; };
struct detail_result {
    source_key source;
    seq::sequence_emit_counts counts{};
    std::uint64_t examined = 0, invalid = 0, excluded_halos = 0;
};

class hierarchy {
    struct node { span region; summary value; std::size_t left, right; bool leaf; };
    struct forest { std::vector<node> nodes; std::size_t root = 0; };
    source_owner source_;
    vocabulary vocabulary_;
    std::vector<forest> forests_;
    std::vector<std::uint32_t> reservoir_positions_;
    std::vector<ce_moon::LiftPair> reservoir_;
    bool odd_tail_ = false;
    double tail_ = 0;
    std::size_t inspected_bases_ = 0;

    static summary join(const summary& left, const summary& right) {
        summary value;
        for (std::size_t i = 0; i < 4; ++i) value.counts[i] = left.counts[i] + right.counts[i];
        value.invalid = left.invalid + right.invalid;
        value.ordered = ce_moon::compose(left.ordered, right.ordered);
        value.ordered.validate();
        return value;
    }
    summary scan(span region) const {
        summary value;
        for (auto p = region.begin; p < region.end; ++p) {
            const auto base = source_->base(p);
            if (base < 0) ++value.invalid; else ++value.counts[base];
            value.ordered = ce_moon::compose(value.ordered, vocabulary_[base < 0 ? 4 : base]);
        }
        value.ordered.validate();
        return value;
    }
    std::size_t build(forest& tree, const std::vector<span>& regions, std::size_t begin, std::size_t end) {
        if (end - begin == 1) {
            const auto region = regions[begin];
            tree.nodes.push_back({region, scan(region), 0, 0, true});
            inspected_bases_ += region.end - region.begin;
        } else {
            const auto middle = begin + (end - begin) / 2;
            const auto left = build(tree, regions, begin, middle), right = build(tree, regions, middle, end);
            tree.nodes.push_back({{tree.nodes[left].region.begin, tree.nodes[right].region.end},
                join(tree.nodes[left].value, tree.nodes[right].value), left, right, false});
        }
        return tree.nodes.size() - 1;
    }
    query_result query_node(const forest& tree, std::size_t id, span support) const {
        const auto& current = tree.nodes[id];
        if (support.begin <= current.region.begin && support.end >= current.region.end)
            return {current.value, 1, 0};
        if (current.region.end <= support.begin || current.region.begin >= support.end) return {};
        if (current.leaf) {
            const span clipped{std::max(current.region.begin, support.begin),
                std::min(current.region.end, support.end)};
            return {scan(clipped), 0, clipped.end - clipped.begin};
        }
        const auto left = query_node(tree, current.left, support);
        const auto right = query_node(tree, current.right, support);
        return {join(left.value, right.value), left.summary_nodes + right.summary_nodes,
            left.replayed_bases + right.replayed_bases};
    }
public:
    hierarchy(source_owner source, vocabulary operations,
              const std::vector<std::vector<std::uint32_t>>& alternatives)
        : source_(std::move(source)), vocabulary_(std::move(operations)) {
        if (!source_ || alternatives.empty()) throw std::invalid_argument("hierarchy ownership/boundaries");
        for (const auto& operation : vocabulary_) operation.validate();
        for (const auto& cuts : alternatives) {
            forest tree;
            std::vector<span> regions;
            std::uint32_t begin = 0;
            for (const auto end : cuts) {
                if (end <= begin || end >= source_->size()) throw std::invalid_argument("chunk cuts");
                regions.push_back({begin, end}); begin = end;
            }
            if (begin < source_->size()) regions.push_back({begin, source_->size()});
            if (!regions.empty()) tree.root = build(tree, regions, 0, regions.size());
            forests_.push_back(std::move(tree));
        }
        std::vector<double> carriers;
        for (std::uint32_t p = 0; p < source_->size(); ++p) {
            const auto base = source_->base(p);
            if (base < 0) continue;
            reservoir_positions_.push_back(p);
            carriers.push_back(base == 1 || base == 2 ? 1. : -1.);
        }
        inspected_bases_ += source_->size();
        for (std::size_t i = 0; i + 1 < carriers.size(); i += 2)
            reservoir_.push_back(ce_moon::lift(carriers[i], carriers[i + 1]));
        odd_tail_ = carriers.size() % 2;
        if (odd_tail_) tail_ = carriers.back();
    }
    const source_owner& source() const noexcept { return source_; }
    std::size_t forests() const noexcept { return forests_.size(); }
    std::size_t inspected_bases() const noexcept { return inspected_bases_; }
    query_result query(source_key expected, span support, std::size_t alternative = 0) const {
        if (!(expected == source_->key())) throw std::invalid_argument("stale hierarchy query");
        source_->check(support);
        if (alternative >= forests_.size()) throw std::out_of_range("hierarchy alternative");
        if (support.begin == support.end) return {};
        const auto& tree = forests_[alternative];
        return query_node(tree, tree.root, support);
    }
    state response(source_key expected, span support, state entry, std::size_t alternative = 0) const {
        for (const auto value : entry) if (!std::isfinite(value)) throw std::invalid_argument("response entry");
        const auto answer = ce_moon::apply(query(expected, support, alternative).value.ordered, entry);
        for (const auto value : answer) if (!std::isfinite(value)) throw std::overflow_error("response output");
        return answer;
    }
    // Scalar GC/AT carriers only, not a claim of reversible arbitrary embeddings.
    // Dropping residuals explicitly returns a lossy coarse view.
    std::vector<double> carriers(bool include_detail) const {
        std::vector<double> values;
        for (const auto pair : reservoir_) {
            const auto expanded = ce_moon::unlift({pair.coarse, include_detail ? pair.detail : 0});
            values.push_back(expanded.first); values.push_back(expanded.second);
        }
        if (odd_tail_) values.push_back(tail_);
        return values;
    }
    const std::vector<std::uint32_t>& carrier_positions() const noexcept { return reservoir_positions_; }
    std::size_t residual_bytes() const noexcept { return reservoir_.size() * sizeof(double); }
    detail_result revisit(source_key expected, span support, detail* output,
                          std::uint64_t capacity, bool include_halos = false) const {
        if (!(expected == source_->key())) throw std::invalid_argument("stale source revisit");
        source_->check(support);
        if (capacity && !output) throw std::invalid_argument("detail output");
        detail_result result{source_->key()};
        for (auto p = support.begin; p < support.end; ++p) {
            ++result.examined;
            const auto base = source_->base(p);
            if (base < 0) { ++result.invalid; continue; }
            const auto owned = source_->owned(p);
            if (!owned && !include_halos) { ++result.excluded_halos; continue; }
            ++result.counts.total_matches;
            if (result.counts.stored_records < capacity)
                output[result.counts.stored_records++] = {source_->coordinate(p), p,
                    static_cast<std::uint8_t>(base), owned};
        }
        result.counts.required_capacity = result.counts.total_matches;
        result.counts.dropped_records = result.counts.total_matches - result.counts.stored_records;
        return result;
    }
};

} // namespace baseplane::representation
