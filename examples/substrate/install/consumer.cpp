#include <Baseplane/seq/dna2_validity.hh>
#include <iostream>
#include <stdexcept>
#if BP_REQUIRE_INTEGRATED
#include <Baseplane/query/contracts/sequence_question.hh>
#include <Baseplane/representation/hierarchy.hh>
#include <Baseplane/index/sequence_index.hh>
#include <Baseplane/incremental/reuse.hh>
#include <Baseplane/learning/sequence_routes.hh>
#endif
static void check(bool value) { if (!value) throw std::runtime_error("installed consumer"); }
int main() {
    namespace seq = baseplane::seq;
    const std::uint64_t words[] = {36}, valid[] = {7};
    const seq::dna2_packed64_valid_view packed{{words, 3, 1}, valid, 1};
    const seq::dna2_chunk_coordinates coordinates{{17, 4, 2}, 100, 3, 0, 3, 0, 0};
    check(seq::dna2_valid_view(packed) && seq::dna2_window_is_valid(packed, 0, 3)
        && !seq::dna2_base_is_valid(packed, 3));
    std::cout << "installed_exact_core_cuda=" << BASEPLANE_ENABLE_CUDA << '\n';
#if BP_REQUIRE_INTEGRATED
    namespace query = baseplane::query;
    const seq::source_view source{packed, coordinates, 1, 1};
    const query::sequence_question question{seq::stamp(source), 0, 3,
        seq::source_strand::forward, query::support_domain::owned};
    query::sequence_answer answer{};
    check(query::count_valid_source(source, question, &answer)
        && std::get<query::exact_count>(answer.payload).value == 3);
    namespace rep = baseplane::representation;
    auto snapshot = std::make_shared<const rep::exact_source>(packed, coordinates, 1, 1);
    rep::vocabulary operations;
    for (auto& operation : operations) { operation = rep::effect::identity(); operation.b = {1, 0}; }
    rep::hierarchy hierarchy(snapshot, operations, {{1}, {2}});
    check(hierarchy.response(snapshot->key(), {0, 3}, {2, 3}) == rep::state({5, 3}));
    namespace index = baseplane::index;
    std::vector<index::source_input> sources;
    for (unsigned role = 0; role < 3; ++role) sources.push_back({packed, coordinates, 1, 1, index::strand::forward, role});
    const index::sequence_index directory(sources, 1);
    const auto range = directory.equal_range("C");
    check(range.second - range.first == 3);
    std::array<std::vector<index::factor_posting>, 3> postings;
    for (const auto& object : directory.objects()) {
        const auto role = object.support.source.role;
        postings[role].push_back({object.id, 7, double(role + 1)});
    }
    index::factor factor{};
    const auto factors = index::join_factors(directory, postings, {0, 1, 1, 1}, &factor, 1);
    check(factors.counts.required == 27 && factors.counts.stored == 1 && factor.score == 6);
    namespace reuse = baseplane::incremental;
    const reuse::source_snapshot text{"ACG", coordinates, 1, 1};
    const reuse::sequence_state state(text, 2, 2);
    reuse::invalidation invalidated{};
    const auto edited = state.edited(text.stamp(), {1, 1, "A"}, &invalidated);
    check(edited.source().exact == "AAG" && edited.directory().count("AC") == 0
        && edited.directory().count("AA") == 1);
    namespace learning = baseplane::learning;
    std::array<bool, 8> observed{}; observed.fill(true);
    const auto model = learning::fit(observed, 11, 12, 13, 14, 20);
    const learning::grounded_sequence input{"ACG", coordinates, 1, 1};
    const auto learned = learning::encode(input, model, learning::route_kind::hardened);
    const auto constructed = learning::encode(input, model, learning::route_kind::constructed);
    check(learned.gates.size() == 1 && learned.gates[0].selected == constructed.gates[0].selected);
    std::cout << "installed integrated query/hierarchy/index/reuse/learning consumer passed\n";
#else
    (void)coordinates;
    std::cout << "installed standalone exact sequence consumer passed\n";
#endif
}
