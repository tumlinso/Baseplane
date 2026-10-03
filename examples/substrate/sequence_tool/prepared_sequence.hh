#pragma once
#include <Baseplane/seq/predicate_plan.hh>
#include <Baseplane/query/contracts/sequence_question.hh>
#include <Baseplane/representation/hierarchy.hh>
#include <Baseplane/index/sequence_index.hh>
#include <Baseplane/incremental/reuse.hh>
#include <Baseplane/learning/sequence_routes.hh>

namespace sequence_tool {
namespace seq = baseplane::seq;
namespace query = baseplane::query;
namespace rep = baseplane::representation;
namespace idx = baseplane::index;
namespace reuse = baseplane::incremental;
namespace learn = baseplane::learning;

// One immutable prepared snapshot. Queries borrow only caller-owned output buffers.
// This example declares forward, owned support, GC predicates and binary64 effects.
class prepared_sequence {
    reuse::sequence_state state_;
    std::vector<std::uint64_t> packed_, valid_;
    rep::vocabulary vocabulary_;
    learn::route_model model_;
    seq::prepared_predicate_plan predicate_{};
    std::unique_ptr<rep::hierarchy> hierarchy_;
    std::unique_ptr<idx::sequence_index> index_;
    void prepare() {
        const auto n = state_.source().coordinates.base_count;
        packed_.assign(seq::dna2_packed64_word_count(n), 0);
        valid_.assign(seq::dna2_validity64_word_count(n), 0);
        for (std::uint32_t p = 0; p < n; ++p) {
            const auto base = seq::dna2_encode_base_with_validity(state_.source().exact[p]);
            packed_[p / 32] |= std::uint64_t{base.code} << (2 * (p % 32));
            if (base.valid) valid_[p / 64] |= std::uint64_t{1} << (p % 64);
        }
        seq::sequence_predicate_program program{};
        program.version = seq::sequence_program_version;
        program.node_count = program.output_count = 1;
        program.nodes[0] = {seq::predicate_opcode::gc_mask, seq::predicate_value_kind::mask,
            0, 0, seq::sequence_program_no_input, seq::sequence_program_no_input, 0, 0};
        program.outputs[0] = {0, 1, seq::sequence_output_mode::count, 0, 0, 0};
        if (seq::prepare_sequence_predicate_program(program, &predicate_) != seq::predicate_plan_status::ok)
            throw std::invalid_argument("GC predicate preparation");
        const auto source = view();
        auto exact = std::make_shared<const rep::exact_source>(source.sequence, source.coordinates,
            source.structure_epoch, source.value_generation);
        const auto encoded = learn::encode(grounded(), model_, learn::route_kind::hardened, 2, 5);
        std::vector<std::uint32_t> cuts;
        for (const auto& chunk : encoded.chunks) if (chunk.end < n) cuts.push_back(chunk.end);
        hierarchy_ = std::make_unique<rep::hierarchy>(exact, vocabulary_,
            std::vector<std::vector<std::uint32_t>>{{}, cuts});
        index_ = std::make_unique<idx::sequence_index>(std::vector<idx::source_input>{
            {source.sequence, source.coordinates, source.structure_epoch, source.value_generation}}, 2);
    }
    learn::grounded_sequence grounded() const {
        const auto& s = state_.source();
        return {s.exact, s.coordinates, s.structure_epoch, s.value_generation};
    }
public:
    struct request {
        query::sequence_question question;
        rep::state entry{0, 0};
        std::size_t alternative = 0;
        learn::route_kind route = learn::route_kind::constructed;
        bool include_carrier_detail = true;
    };
    struct result {
        query::sequence_answer valid_count;
        rep::query_result summary;
        rep::state response;
        rep::detail_result detail;
        std::uint64_t gc_matches;
        idx::capacity_counts nonlocal_pairs;
        learn::representation route;
        std::vector<double> carriers;
        bool carriers_lossy;
    };
    prepared_sequence(reuse::source_snapshot source, rep::vocabulary vocabulary, learn::route_model model)
        : state_(std::move(source), 3, 2), vocabulary_(std::move(vocabulary)), model_(std::move(model)) {
        if (state_.source().orientation != reuse::strand::forward)
            throw std::invalid_argument("example requires forward source");
        prepare();
    }
    seq::source_view view() const {
        const auto& s = state_.source();
        return {{{packed_.data(), s.coordinates.base_count, packed_.size()}, valid_.data(), valid_.size()},
            s.coordinates, s.structure_epoch, s.value_generation};
    }
    query::sequence_question question(std::uint32_t begin, std::uint32_t length) const {
        return {seq::stamp(view()), begin, length, seq::source_strand::forward, query::support_domain::owned};
    }
    const idx::object& nonlocal_object(std::uint64_t id) const { return index_->at(id); }
    const reuse::sequence_state& incremental() const { return state_; }
    const seq::prepared_predicate_plan& predicate() const { return predicate_; }
    result execute(const request& request, rep::detail* detail, std::uint64_t detail_capacity,
                   idx::pair* pairs, std::uint64_t pair_capacity) const {
        const auto source = view();
        if (query::validate_question(source, request.question) != query::question_status::ok
            || request.question.strand != seq::source_strand::forward
            || request.question.domain != query::support_domain::owned)
            throw std::invalid_argument("stale/unsupported sequence question");
        if ((detail_capacity && !detail) || (pair_capacity && !pairs))
            throw std::invalid_argument("caller output capacity");
        const rep::span support{request.question.begin, request.question.begin + request.question.length};
        // Effects and carriers cover declared support; this example disallows halo effects.
        if (support.begin < source.coordinates.owned_begin || support.end > source.coordinates.owned_end)
            throw std::invalid_argument("effects require owned support");
        query::sequence_answer count{};
        if (!query::count_valid_source(source, request.question, &count))
            throw std::invalid_argument("exact count question");
        auto route = learn::encode(grounded(), model_, request.route, 2, 5);
        route.gates.erase(std::remove_if(route.gates.begin(), route.gates.end(), [&](const auto& gate) {
            return gate.anchor < support.begin || gate.anchor >= support.end;
        }), route.gates.end());
        const auto key = hierarchy_->source()->key();
        const auto summary = hierarchy_->query(key, support, request.alternative);
        // Execute the existing native host GC predicate, masking validity and tail/support.
        std::uint64_t gc = 0;
        for (std::uint32_t word = support.begin / 32; word < (support.end + 31ull) / 32; ++word) {
            const auto mask = seq::planes_gc_mask(seq::unpack_word64_to_planes32({packed_[word]}));
            for (std::uint32_t bit = 0; bit < 32; ++bit) {
                const auto p = word * 32 + bit;
                if (p >= support.begin && p < support.end && (valid_[p / 64] & (std::uint64_t{1} << (p % 64))))
                    gc += (mask >> bit) & 1;
            }
        }
        auto carriers = hierarchy_->carriers(request.include_carrier_detail);
        const auto& positions = hierarchy_->carrier_positions();
        std::vector<double> selected;
        for (std::size_t i = 0; i < positions.size(); ++i)
            if (positions[i] >= support.begin && positions[i] < support.end) selected.push_back(carriers[i]);
        // Nonlocal group is explicitly the full prepared owned source, separate from local question support.
        return {count, summary, hierarchy_->response(key, support, request.entry, request.alternative),
            hierarchy_->revisit(key, support, detail, detail_capacity), gc,
            index_->equal_pairs(pairs, pair_capacity), std::move(route), std::move(selected),
            !request.include_carrier_detail};
    }
    prepared_sequence edited(reuse::source_stamp expected, const reuse::text_edit& edit,
                             reuse::invalidation* invalidated) const {
        const auto next = state_.edited(expected, edit, invalidated);
        // Incremental owner repairs values/postings. Derived hierarchy/learned/index views rebuild explicitly.
        return prepared_sequence(next.source(), vocabulary_, model_);
    }
};
} // namespace sequence_tool
