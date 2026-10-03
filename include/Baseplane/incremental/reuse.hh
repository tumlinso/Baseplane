#pragma once
#include <Baseplane/seq/dna2_validity.hh>
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace baseplane::incremental {
using u64 = std::uint64_t;
enum class strand : std::uint8_t { forward, reverse };
struct source_stamp {
    seq::dna2_chunk_identity identity;
    u64 structure_epoch, value_generation, origin;
    std::uint32_t length;
    strand orientation;
    auto tuple() const {
        return std::tie(identity.genome, identity.contig, identity.chunk, structure_epoch,
            value_generation, origin, length, orientation);
    }
};
inline bool operator==(const source_stamp& a, const source_stamp& b) { return a.tuple() == b.tuple(); }
inline bool operator!=(const source_stamp& a, const source_stamp& b) { return !(a == b); }

struct source_snapshot {
    std::string exact;
    seq::dna2_chunk_coordinates coordinates;
    u64 structure_epoch, value_generation;
    strand orientation = strand::forward;
    source_stamp stamp() const {
        return {coordinates.identity, structure_epoch, value_generation, coordinates.global_base_begin,
            coordinates.base_count, orientation};
    }
    void validate() const {
        if (!seq::dna2_valid_chunk_coordinates(coordinates) || exact.size() != coordinates.base_count
            || (orientation != strand::forward && orientation != strand::reverse))
            throw std::invalid_argument("incremental source");
    }
};
struct support { std::uint32_t begin, end; };
inline u64 gc_count(const std::string& text, support range) {
    if (range.begin > range.end || range.end > text.size()) throw std::out_of_range("count support");
    u64 count = 0;
    for (auto p = range.begin; p < range.end; ++p) {
        const auto base = seq::dna2_encode_base_with_validity(text[p]);
        count += base.valid && (base.code == 1 || base.code == 2);
    }
    return count;
}
// Invalid original symbols remain exact content tokens. No packed-payload
// equality is used to intern content; occurrences never live inside shared work.
struct content { std::string exact; u64 gc; };
class content_pool {
    std::map<u64, std::vector<std::shared_ptr<const content>>> candidates_;
    std::size_t created_ = 0;
public:
    std::shared_ptr<const content> intern(const std::string& text, u64 nomination) {
        auto& candidates = candidates_[nomination];
        for (const auto& candidate : candidates) if (candidate->exact == text) return candidate;
        if (text.size() > seq::dna2_max_local_base_count) throw std::length_error("content size");
        auto item = std::make_shared<const content>(content{text,
            gc_count(text, {0, static_cast<std::uint32_t>(text.size())})});
        candidates.push_back(item); ++created_; return item;
    }
    std::size_t created() const noexcept { return created_; }
};

// IDs name complete caller-declared query/parameter/state interpretations.
// A different world is distinct even if its exact content currently equals base.
struct guard_key {
    source_stamp source;
    u64 occurrence, context_generation, model, weights_generation;
    u64 representation_generation, numerical_policy, world, world_generation, query;
    support inspected;
    auto tuple() const {
        return std::tuple_cat(source.tuple(), std::tie(occurrence, context_generation, model,
            weights_generation, representation_generation, numerical_policy, world,
            world_generation, query, inspected.begin, inspected.end));
    }
    void validate() const {
        if (inspected.begin > inspected.end || inspected.end > source.length)
            throw std::invalid_argument("guard support");
        if (source.orientation != strand::forward && source.orientation != strand::reverse)
            throw std::invalid_argument("guard strand");
        if (source.length && source.origin > std::numeric_limits<u64>::max() - (source.length - 1))
            throw std::invalid_argument("guard coordinates");
    }
};
inline bool operator==(const guard_key& a, const guard_key& b) { return a.tuple() == b.tuple(); }
inline bool operator<(const guard_key& a, const guard_key& b) { return a.tuple() < b.tuple(); }
struct occurrence {
    std::shared_ptr<const content> shared_content;
    guard_key guard;
    double contextual_residual;
    occurrence(std::shared_ptr<const content> shared, guard_key guard, double residual)
        : shared_content(std::move(shared)), guard(guard), contextual_residual(residual) {
        this->guard.validate();
        if (!shared_content || shared_content->exact.size() != guard.source.length || !std::isfinite(residual))
            throw std::invalid_argument("contextual occurrence");
    }
};
enum class artifact_kind { result, plan, residual };
// Separate typed stores share key rules; plans never implicitly return answers.
template<artifact_kind Kind, class T> class guarded_store {
    std::map<guard_key, T> values_;
public:
    void put(guard_key key, T value) { key.validate(); values_.insert_or_assign(key, std::move(value)); }
    const T* find(const guard_key& key) const {
        key.validate(); const auto found = values_.find(key);
        return found == values_.end() ? nullptr : &found->second;
    }
};
template<class T> using result_store = guarded_store<artifact_kind::result, T>;
template<class T> using plan_store = guarded_store<artifact_kind::plan, T>;
template<class T> using residual_store = guarded_store<artifact_kind::residual, T>;

struct text_edit { std::uint32_t position, removed; std::string inserted; };
struct edit_record {
    u64 from_generation, to_generation;
    std::uint32_t old_length, position, removed, inserted;
};
struct invalidation {
    source_stamp before, after;
    edit_record edit;
    std::vector<std::size_t> values, memberships;
    bool rebuilt = false;
};
class sequence_state {
    source_snapshot source_;
    std::uint32_t chunk_size_, window_;
    std::vector<u64> values_;
    std::map<std::string, std::set<std::size_t>> directory_;
    std::string window_key(std::size_t p) const {
        std::string key;
        for (std::size_t j = 0; j < window_; ++j) {
            const auto base = seq::dna2_encode_base_with_validity(source_.exact[p + j]);
            if (!base.valid) return {};
            key += "ACGT"[base.code];
        }
        return key;
    }
    bool owned(std::size_t logical) const {
        const auto physical = source_.orientation == strand::forward ? logical
            : source_.coordinates.base_count - 1 - logical;
        return seq::dna2_anchor_is_owned(source_.coordinates, static_cast<std::uint32_t>(physical));
    }
    std::size_t windows() const noexcept {
        return source_.exact.size() < window_ ? 0 : source_.exact.size() - window_ + 1;
    }
    void rebuild() {
        values_.clear(); directory_.clear();
        for (std::size_t p = 0; p < source_.exact.size(); p += chunk_size_)
            values_.push_back(gc_count(source_.exact, {static_cast<std::uint32_t>(p),
                static_cast<std::uint32_t>(std::min(source_.exact.size(), p + chunk_size_))}));
        for (std::size_t p = 0; p < windows(); ++p) {
            if (!owned(p)) continue;
            const auto key = window_key(p);
            if (!key.empty()) directory_[key].insert(p);
        }
    }
public:
    sequence_state(source_snapshot source, std::uint32_t chunk_size, std::uint32_t window)
        : source_(std::move(source)), chunk_size_(chunk_size), window_(window) {
        source_.validate();
        if (!chunk_size || !window) throw std::invalid_argument("incremental partition");
        rebuild();
    }
    const source_snapshot& source() const noexcept { return source_; }
    const std::vector<u64>& values() const noexcept { return values_; }
    const std::map<std::string, std::set<std::size_t>>& directory() const noexcept { return directory_; }
    sequence_state edited(source_stamp expected, const text_edit& edit, invalidation* receipt) const {
        if (!receipt || expected != source_.stamp()) throw std::invalid_argument("edit source version/receipt");
        const auto n = source_.exact.size();
        if (edit.position > n || edit.removed > n - edit.position
            || edit.inserted.size() > seq::dna2_max_local_base_count - (n - edit.removed))
            throw std::out_of_range("edit support");
        if (source_.value_generation == std::numeric_limits<u64>::max())
            throw std::overflow_error("source generation");
        sequence_state next = *this;
        next.source_.exact.replace(edit.position, edit.removed, edit.inserted);
        ++next.source_.value_generation;
        invalidation changed{source_.stamp(), {}, {source_.value_generation, next.source_.value_generation,
            static_cast<std::uint32_t>(n), edit.position, edit.removed,
            static_cast<std::uint32_t>(edit.inserted.size())}, {}, {}, false};
        if (edit.removed != edit.inserted.size()) {
            if (source_.structure_epoch == std::numeric_limits<u64>::max())
                throw std::overflow_error("structure epoch");
            ++next.source_.structure_epoch;
            const auto length = static_cast<std::uint32_t>(next.source_.exact.size());
            if (length < source_.coordinates.halo_left + source_.coordinates.halo_right)
                throw std::invalid_argument("edit would remove declared halos");
            next.source_.coordinates.base_count = length;
            next.source_.coordinates.owned_end = length - source_.coordinates.halo_right;
            next.source_.validate(); next.rebuild(); changed.rebuilt = true;
            for (std::size_t i = 0; i < std::max(values_.size(), next.values_.size()); ++i) changed.values.push_back(i);
            for (std::size_t p = 0; p < std::max(windows(), next.windows()); ++p) changed.memberships.push_back(p);
        } else {
            const std::size_t end = edit.position + edit.removed;
            for (std::size_t i = 0; i < values_.size(); ++i) {
                const auto begin = i * chunk_size_, stop = std::min(n, begin + chunk_size_);
                if (begin >= end || stop <= edit.position) continue;
                changed.values.push_back(i);
                next.values_[i] = gc_count(next.source_.exact,
                    {static_cast<std::uint32_t>(begin), static_cast<std::uint32_t>(stop)});
            }
            for (std::size_t p = 0; p < windows(); ++p) {
                if (p >= end || p + window_ <= edit.position || !owned(p)) continue;
                changed.memberships.push_back(p);
                const auto old = window_key(p), current = next.window_key(p);
                if (!old.empty()) {
                    auto found = next.directory_.find(old);
                    if (found == next.directory_.end() || !found->second.erase(p))
                        throw std::logic_error("missing old posting");
                    if (found->second.empty()) next.directory_.erase(found);
                }
                if (!current.empty()) next.directory_[current].insert(p);
            }
        }
        changed.after = next.source_.stamp(); *receipt = std::move(changed); return next;
    }
};

// Support covers all inspected bases, including negative evidence. Only a
// complete consecutive edit chain can justify cross-generation result reuse.
inline bool reusable(guard_key saved, const guard_key& current, const std::vector<edit_record>& edits) {
    saved.validate(); current.validate();
    const auto old_generation = saved.source.value_generation;
    saved.source.value_generation = current.source.value_generation;
    if (!(saved == current)) return false; // Resizes/epochs, worlds and guards are conservative misses.
    if (old_generation == current.source.value_generation) return edits.empty();
    u64 generation = old_generation;
    for (const auto& edit : edits) {
        if (generation == std::numeric_limits<u64>::max() || edit.from_generation != generation
            || edit.to_generation != generation + 1 || edit.old_length != saved.source.length
            || edit.position > edit.old_length || edit.removed > edit.old_length - edit.position
            || edit.removed != edit.inserted) return false;
        if (edit.position < saved.inspected.end
            && edit.position + edit.removed > saved.inspected.begin) return false;
        generation = edit.to_generation;
    }
    return generation == current.source.value_generation;
}

struct world_delta {
    guard_key baseline;
    u64 target_world, target_generation;
    std::vector<text_edit> edits;
};
struct world_result { sequence_state state; guard_key guard, baseline; std::vector<invalidation> invalidated; };
inline world_result derive_world(const sequence_state& base, const guard_key& current, const world_delta& delta) {
    current.validate(); delta.baseline.validate();
    if (!(current == delta.baseline) || current.source != base.source().stamp()
        || delta.target_world == current.world)
        throw std::invalid_argument("world delta baseline/identity");
    world_result result{base, current, current, {}};
    for (const auto& edit : delta.edits) {
        invalidation changed{};
        result.state = result.state.edited(result.state.source().stamp(), edit, &changed);
        result.invalidated.push_back(std::move(changed));
    }
    result.guard.source = result.state.source().stamp();
    result.guard.world = delta.target_world; result.guard.world_generation = delta.target_generation;
    result.guard.validate(); return result;
}

} // namespace baseplane::incremental
