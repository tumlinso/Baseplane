#pragma once
#include <Baseplane/seq/dna2_ops.hh>
#include <Baseplane/seq/dna2_validity.hh>
#include <ce_moon/mechanisms.hpp>
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace baseplane::index {
using u64 = std::uint64_t;
enum class strand : std::uint8_t { forward, reverse };
struct source_id {
    seq::dna2_chunk_identity identity;
    u64 structure_epoch, value_generation;
    strand orientation;
    std::uint32_t role;
};
inline bool operator==(const source_id& a, const source_id& b) noexcept {
    return a.identity.genome == b.identity.genome && a.identity.contig == b.identity.contig
        && a.identity.chunk == b.identity.chunk && a.structure_epoch == b.structure_epoch
        && a.value_generation == b.value_generation && a.orientation == b.orientation && a.role == b.role;
}
struct source_input {
    seq::dna2_packed64_valid_view sequence;
    seq::dna2_chunk_coordinates coordinates;
    u64 structure_epoch, value_generation;
    strand orientation = strand::forward;
    std::uint32_t role = 0;
};
struct source_support { source_id source; u64 first, last; std::uint32_t logical_begin; };
struct object { u64 id; std::string exact; source_support support; };
struct pair { u64 first, second; };
struct capacity_counts { u64 required = 0, stored = 0, dropped = 0; };
inline u64 add(u64 a, u64 b) {
    if (b > std::numeric_limits<u64>::max() - a) throw std::overflow_error("index count");
    return a + b;
}
inline u64 product(u64 a, u64 b) {
    if (a && b > std::numeric_limits<u64>::max() / a) throw std::overflow_error("index cardinality");
    return a * b;
}
inline u64 combinations(u64 n) { return n < 2 ? 0 : (n % 2 ? product(n, (n - 1) / 2) : product(n / 2, n - 1)); }
inline std::string canonical(std::string key) {
    if (key.empty()) throw std::invalid_argument("empty key");
    for (auto& c : key) {
        const auto encoded = seq::dna2_encode_base_with_validity(c);
        if (!encoded.valid) throw std::invalid_argument("invalid key");
        c = seq::base_to_char(encoded.code);
    }
    return key;
}
struct nomination {
    std::vector<pair> candidates;
    capacity_counts counts;
    unsigned probes = 0;
};
struct recall_evidence { u64 truth = 0, nominated_truth = 0, missed = 0, comparisons = 0; };
struct verification { pair candidate; std::size_t distance; bool accepted; };
struct scored_relation { verification exact; double score; };
struct subscription {
    std::string motif;
    source_id expected_source;
    u64 destination;
    std::uint32_t question_role;
    u64 begin, end; // inclusive genomic bounds; supports at UINT64_MAX remain representable.
};
struct subscription_answer { u64 destination, object_id; std::uint32_t question_role; source_support support; };

class sequence_index {
    struct posting { std::string key; u64 id; };
    std::size_t width_;
    std::vector<object> objects_;
    std::vector<posting> postings_;
    std::vector<source_id> sources_;
    u64 inspected_ = 0;
    using groups = std::map<std::string, std::vector<u64>>;
    groups sketch_groups(unsigned probe) const {
        groups result;
        for (const auto& object : objects_) {
            const auto middle = width_ / 2;
            result[probe == 0 ? object.exact.substr(0, middle) : object.exact.substr(middle)].push_back(object.id);
        }
        return result;
    }
    static u64 group_pairs(const groups& input) {
        u64 count = 0;
        for (const auto& group : input) count = add(count, combinations(group.second.size()));
        return count;
    }
public:
    sequence_index(const std::vector<source_input>& inputs, std::size_t width) : width_(width) {
        if (!width) throw std::invalid_argument("zero window");
        for (const auto& source : inputs) {
            if (!seq::dna2_valid_view(source.sequence) || !seq::dna2_valid_chunk_coordinates(source.coordinates)
                || source.sequence.packed.n_bases != source.coordinates.base_count
                || (source.orientation != strand::forward && source.orientation != strand::reverse))
                throw std::invalid_argument("index source");
            const source_id identity{source.coordinates.identity, source.structure_epoch,
                source.value_generation, source.orientation, source.role};
            if (std::find(sources_.begin(), sources_.end(), identity) != sources_.end())
                throw std::invalid_argument("duplicate source descriptor");
            sources_.push_back(identity);
            const auto n = source.coordinates.base_count;
            if (width > n) continue;
            const auto physical = [&](std::uint32_t logical) {
                return source.orientation == strand::forward ? logical : n - 1 - logical;
            };
            for (std::uint32_t p = 0; p <= n - width; ++p) {
                // Halo windows may provide context, but ownership of the logical
                // anchor controls whether this chunk emits an object.
                if (!seq::dna2_anchor_is_owned(source.coordinates, physical(p))) continue;
                std::string key;
                bool valid = true;
                for (std::size_t j = 0; j < width; ++j) {
                    const auto position = physical(static_cast<std::uint32_t>(p + j));
                    inspected_ = add(inspected_, 1);
                    if (!seq::dna2_base_is_valid(source.sequence, position)) { valid = false; continue; }
                    auto code = seq::get_base(seq::dna2_word64{source.sequence.packed.words[position / 32]},
                        static_cast<int>(position % 32));
                    if (source.orientation == strand::reverse) code ^= 3;
                    key.push_back(seq::base_to_char(code));
                }
                if (!valid) continue;
                u64 first = 0, last = 0;
                seq::dna2_local_to_global(source.coordinates, physical(p), &first);
                seq::dna2_local_to_global(source.coordinates, physical(static_cast<std::uint32_t>(p + width - 1)), &last);
                const u64 id = objects_.size();
                objects_.push_back({id, std::move(key), {identity, first, last, p}});
                postings_.push_back({objects_.back().exact, id});
            }
        }
        std::sort(postings_.begin(), postings_.end(), [](const posting& a, const posting& b) {
            return a.key < b.key || (a.key == b.key && a.id < b.id);
        });
    }
    const std::vector<object>& objects() const noexcept { return objects_; }
    u64 inspected_bases() const noexcept { return inspected_; }
    const object& at(u64 id) const { return objects_.at(id); }
    std::pair<std::size_t, std::size_t> equal_range(std::string key) const {
        key = canonical(std::move(key));
        if (key.size() != width_) throw std::invalid_argument("index key width");
        const auto begin = std::lower_bound(postings_.begin(), postings_.end(), key,
            [](const posting& row, const std::string& value) { return row.key < value; });
        const auto end = std::upper_bound(begin, postings_.end(), key,
            [](const std::string& value, const posting& row) { return value < row.key; });
        return {static_cast<std::size_t>(begin - postings_.begin()), static_cast<std::size_t>(end - postings_.begin())};
    }
    const object& posting_at(std::size_t offset) const { return at(postings_.at(offset).id); }
    capacity_counts equal_pairs(pair* output, u64 capacity) const {
        if (capacity && !output) throw std::invalid_argument("pair output");
        capacity_counts result;
        for (std::size_t begin = 0; begin < postings_.size();) {
            auto end = begin + 1;
            while (end < postings_.size() && postings_[end].key == postings_[begin].key) ++end;
            const u64 size = end - begin;
            result.required = add(result.required, product(size, size ? size - 1 : 0));
            // Count complete ranges algebraically even after storage is exhausted.
            for (auto a = begin; a < end && result.stored < capacity; ++a)
                for (auto b = begin; b < end && result.stored < capacity; ++b)
                    if (a != b) output[result.stored++] = {postings_[a].id, postings_[b].id};
            begin = end;
        }
        result.dropped = result.required - result.stored;
        return result;
    }
    nomination nominate(unsigned probes, u64 capacity) const {
        if (width_ < 2 || probes < 1 || probes > 2) throw std::invalid_argument("nomination probes/width");
        nomination result; result.probes = probes;
        const auto first = sketch_groups(0);
        result.counts.required = group_pairs(first);
        groups second;
        if (probes == 2) {
            second = sketch_groups(1);
            groups exact;
            for (const auto& object : objects_) exact[object.exact].push_back(object.id);
            result.counts.required = add(result.counts.required, group_pairs(second) - group_pairs(exact));
        }
        auto emit = [&](const groups& groups, bool skip_first) {
            for (const auto& group : groups) {
                const auto& ids = group.second;
                for (std::size_t a = 0; a < ids.size() && result.candidates.size() < capacity; ++a)
                    for (std::size_t b = a + 1; b < ids.size() && result.candidates.size() < capacity; ++b) {
                        if (skip_first && at(ids[a]).exact.compare(0, width_ / 2,
                            at(ids[b]).exact, 0, width_ / 2) == 0) continue;
                        result.candidates.push_back({ids[a], ids[b]});
                    }
            }
        };
        emit(first, false); if (probes == 2) emit(second, true);
        result.counts.stored = result.candidates.size();
        result.counts.dropped = result.counts.required - result.counts.stored;
        return result;
    }
    verification verify(pair candidate, std::size_t tolerance) const {
        if (candidate.first >= candidate.second) throw std::invalid_argument("nomination pair ordering");
        const auto& a = at(candidate.first).exact; const auto& b = at(candidate.second).exact;
        std::size_t distance = 0;
        for (std::size_t p = 0; p < width_; ++p) distance += a[p] != b[p];
        return {candidate, distance, distance <= tolerance};
    }
    scored_relation score(verification verified, const std::function<double(const object&, const object&)>& scorer,
                          std::size_t tolerance) const {
        const auto exact = verify(verified.candidate, tolerance);
        if (!exact.accepted || !verified.accepted || exact.distance != verified.distance || !scorer)
            throw std::invalid_argument("relation requires exact verification");
        const auto value = scorer(at(exact.candidate.first), at(exact.candidate.second));
        if (!std::isfinite(value)) throw std::invalid_argument("relation score");
        return {exact, value};
    }
    // Evaluation-only exhaustive truth, never used to populate candidates.
    recall_evidence evaluate_recall(const nomination& nominated, std::size_t tolerance) const {
        if (nominated.counts.dropped || nominated.counts.stored != nominated.candidates.size()
            || nominated.counts.required != nominated.candidates.size())
            throw std::invalid_argument("recall requires complete nomination output");
        std::set<std::pair<u64, u64>> candidates;
        for (const auto candidate : nominated.candidates) {
            verify(candidate, tolerance);
            if (!candidates.emplace(candidate.first, candidate.second).second)
                throw std::invalid_argument("duplicate nomination pair");
        }
        recall_evidence result;
        for (u64 a = 0; a < objects_.size(); ++a) for (u64 b = a + 1; b < objects_.size(); ++b) {
            ++result.comparisons;
            if (!verify({a, b}, tolerance).accepted) continue;
            ++result.truth;
            if (candidates.count({a, b})) ++result.nominated_truth; else ++result.missed;
        }
        return result;
    }
    capacity_counts subscribe(const std::vector<subscription>& requests, subscription_answer* output,
                              u64 capacity) const {
        if (capacity && !output) throw std::invalid_argument("subscription output");
        capacity_counts result;
        for (const auto& request : requests) {
            if (request.begin > request.end
                || std::find(sources_.begin(), sources_.end(), request.expected_source) == sources_.end())
                throw std::invalid_argument("subscription support/source version");
            const auto range = equal_range(request.motif);
            for (auto p = range.first; p < range.second; ++p) {
                const auto& object = posting_at(p); const auto& support = object.support;
                if (!(support.source == request.expected_source)
                    || std::min(support.first, support.last) < request.begin
                    || std::max(support.first, support.last) > request.end) continue;
                result.required = add(result.required, 1);
                if (result.stored < capacity)
                    output[result.stored++] = {request.destination, object.id, request.question_role, support};
            }
        }
        result.dropped = result.required - result.stored;
        return result;
    }
};

namespace numeric = ce_moon::mechanisms;
struct factor_posting { u64 object_id, key; double value; };
struct factor { std::array<source_support, 3> roles; std::array<u64, 3> object_ids; u64 key; double score; };
struct factor_result { capacity_counts counts; std::vector<numeric::FactorAggregate> aggregates; };
// Ordered roles 0/1/2 survive the CE opaque ID seam. Compatibility key and
// numeric values are caller declarations; this does not infer biological causality.
inline factor_result join_factors(const sequence_index& index,
    const std::array<std::vector<factor_posting>, 3>& postings, const std::vector<double>& weights,
    factor* output, std::size_t capacity) {
    if (capacity && !output) throw std::invalid_argument("factor output");
    std::array<std::vector<numeric::RoleEntry>, 3> numerical;
    for (std::size_t role = 0; role < 3; ++role) {
        std::set<u64> ids;
        for (const auto& entry : postings[role]) {
            if (index.at(entry.object_id).support.source.role != role || !ids.insert(entry.object_id).second)
                throw std::invalid_argument("factor source role/duplicate");
            numerical[role].push_back({entry.object_id, entry.key, entry.value});
        }
    }
    factor_result result;
    result.aggregates = numeric::aggregate_factors(numerical[0], numerical[1], numerical[2], weights);
    for (const auto& group : result.aggregates) result.counts.required = add(result.counts.required, group.count);
    const auto stored = std::min<u64>(capacity, result.counts.required);
    if (!stored) {
        result.counts.dropped = result.counts.required;
        return result; // Factorized count/score needs no hyperedge traversal.
    }
    std::vector<numeric::FactorTuple> tuples(stored);
    const auto receipt = numeric::join_factors(numerical[0], numerical[1], numerical[2], weights,
        tuples.data(), tuples.size());
    if (receipt.required != result.counts.required) throw std::logic_error("factor provider count mismatch");
    result.counts.stored = receipt.written; result.counts.dropped = result.counts.required - receipt.written;
    for (std::size_t i = 0; i < receipt.written; ++i) {
        const auto& tuple = tuples[i];
        output[i] = {{index.at(tuple.ids[0]).support, index.at(tuple.ids[1]).support, index.at(tuple.ids[2]).support},
            {tuple.ids[0], tuple.ids[1], tuple.ids[2]}, tuple.key, tuple.score};
    }
    return result;
}

} // namespace baseplane::index
