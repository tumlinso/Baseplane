#pragma once
#include <logic/logic.hpp>
#include <automata/automata.hpp>
#include <effects/response.hpp>
#include <demand/demand.hpp>

namespace bp_moon::compositions::sequence_effects {
struct Question {
    unsigned finite_entry;
    response::State floating_entry;
    double refinement_cutoff;
    std::size_t wave_capacity, work_budget;
    u64 source_version;
};
struct Answer {
    SourceMap source;
    unsigned finite;
    response::State floating;
    bool refinement_requested;
    demand::QueryResult refinement;
};
struct Replay {
    std::vector<demand::Support> selected, deferred;
    std::size_t inspected = 0, produced = 0, stored = 0, dropped = 0;
};

class SequenceEffects {
    PackedFixture original_;
    SourceMap map_;
    std::vector<u32> predicate_words_;
    PackedFixture conditioned_;
    automata::EffectHierarchy finite_;
    response::ResponseHierarchy floating_;
    // Predicate output is consumed by both effect trees. GC bases survive;
    // unselected or invalid positions are explicitly reset positions.
    static PackedFixture condition(const PackedFixture& original, const SourceMap& map,
                                   std::vector<u32>& words) {
        demand::validate(original, map);
        std::string result(original.original.size(), 'N');
        const logic::Circuit gc{0x96, 0xf0, 1}; // XOR two packed base planes, then retain hidden result.
        for (std::size_t word = 0; word < original.words.size(); ++word) {
            const auto planes = original.planes(word);
            const logic::Domain domain{map.source_id, map.contig, map.version, word};
            const auto selected = gc.apply({domain, planes.lo, planes.valid},
                                           {domain, planes.hi, planes.valid}, {domain, 0, planes.valid});
            const auto exact_gc = logic::base_mask(original, word, 'C') | logic::base_mask(original, word, 'G');
            if (selected.bits != exact_gc) throw std::logic_error("GC circuit interpretation");
            words.push_back(selected.bits);
            for (unsigned lane = 0; lane < 32; ++lane) {
                if (selected.bits & (u32{1} << lane)) {
                    const auto position = word * 32 + lane;
                    result[position] = automata::canonical(original.original[position]);
                }
            }
        }
        return PackedFixture(std::move(result));
    }
public:
    SequenceEffects(PackedFixture original, SourceMap map)
        : original_(std::move(original)), map_(map), conditioned_(condition(original_, map_, predicate_words_)),
          finite_(conditioned_), floating_(conditioned_, map_) {}
    const SourceMap& source() const { return map_; }
    const std::vector<u32>& predicates() const { return predicate_words_; }
    const PackedFixture& conditioned_source() const { return conditioned_; }
    Answer query(const Question& question) const {
        if (question.finite_entry >= 32 || question.source_version != map_.version ||
            !std::isfinite(question.refinement_cutoff)) throw std::invalid_argument("C01 query domain");
        for (auto value : question.floating_entry)
            if (!std::isfinite(value)) throw std::invalid_argument("C01 floating entry");
        demand::bounded(question.wave_capacity);
        Answer result{map_, finite_.nodes[finite_.root].effect.to[question.finite_entry],
            ce_moon::apply(floating_.nodes[floating_.root].effect, question.floating_entry), false, {}};
        for (auto value : result.floating) ce_moon::effects::finite(value);
        // Both returned effect answers drive the explicit synthetic revisit policy.
        result.refinement_requested = result.floating[0] > ce_moon::effects::finite(question.refinement_cutoff + result.finite);
        if (result.refinement_requested)
            result.refinement = demand::pull(original_, map_, demand::Question::gc_count, 0,
                question.wave_capacity, question.work_budget, question.source_version);
        return result;
    }
    // Revisit only returned deferred source support. GC interpretation matches
    // the predicate conditioning; capacity overflow returns exact one-base support.
    Replay resume(const std::vector<demand::Support>& deferred, std::size_t capacity) const {
        demand::bounded(deferred.size()); demand::bounded(capacity);
        Replay result;
        if (deferred.empty()) return result;
        if (!map_.length) throw std::invalid_argument("deferred empty source");
        const auto full = demand::support(map_, 0, map_.length);
        for (const auto& support : deferred) {
            if (support.source != full.source || support.contig != full.contig || support.version != full.version ||
                support.begin < full.begin || support.end > full.end || support.begin >= support.end)
                throw std::invalid_argument("C01 foreign deferred support");
        }
        for (std::size_t position = 0; position < map_.length; ++position) {
            const auto coordinate = map_.coordinate(position);
            bool requested = false;
            for (const auto& support : deferred)
                requested |= coordinate >= support.begin && coordinate < support.end;
            if (!requested) continue;
            ++result.inspected;
            if (!original_.is_valid(position)) continue;
            const auto code = demand::code(original_.original[position]);
            if (code != 1 && code != 2) continue;
            ++result.produced;
            const auto support = demand::support(map_, position, position + 1);
            if (result.stored < capacity) { result.selected.push_back(support); ++result.stored; }
            else { result.deferred.push_back(support); ++result.dropped; }
        }
        return result;
    }
};
} // namespace bp_moon::compositions::sequence_effects
