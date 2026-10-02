#include "../source/c01.hpp"
#include <iostream>
#include <set>

using namespace bp_moon;
using namespace bp_moon::compositions::sequence_effects;
static void require(bool condition, const char* message) { if (!condition) throw std::runtime_error(message); }
template<class E, class F> static void rejects(F fn, const char* message) {
    try { fn(); } catch (const E&) { return; } throw std::runtime_error(message);
}
static unsigned finite_scan(std::string_view original, unsigned state) {
    for (char raw : original) {
        const auto base = automata::canonical(raw);
        if (base == 'C') state = (state * 4 + 1) % 16;
        else if (base == 'G') state = (state * 4 + 2) % 16;
        else state = 0;
    }
    return state;
}
static response::State float_scan(std::string_view original, response::State state) {
    for (char raw : original) {
        const auto base = automata::canonical(raw);
        const int code = base == 'C' ? 1 : base == 'G' ? 2 : -1;
        if (code < 0) { state = {}; continue; }
        const auto old = state;
        state[0] = (1 + .125 * code) * old[code == 1 ? 1 : 0] + code + 1;
        state[1] = (1 - .0625 * code) * old[code == 1 ? 0 : 1] - code;
    }
    return state;
}
static void close(const response::State& left, const response::State& right) {
    for (unsigned i = 0; i < 2; ++i) require(std::abs(left[i] - right[i]) < 1e-10 * std::max(1., std::abs(right[i])), "floating oracle");
}
static std::set<u64> support_coordinates(const std::vector<demand::Support>& supports) {
    std::set<u64> result;
    for (auto support : supports) {
        require(support.end == support.begin + 1, "refined singleton support");
        result.insert(support.begin);
    }
    return result;
}
static void full_connected_path(Strand strand) {
    std::string original(33, 'A'); original[30] = 'N'; original[31] = 'c'; original[32] = 'G';
    const PackedFixture packed(original);
    const SourceMap source{1001, 8, 500, 2, strand, original.size()};
    const SequenceEffects pipeline(packed, source);
    require(pipeline.predicates().size() == 2 && pipeline.predicates()[0] == (u32{1} << 31) && pipeline.predicates()[1] == 1,
            "GC predicates cross packed word seam, exclude invalid/tail payload");
    for (unsigned entry = 0; entry < 32; ++entry) {
        const Question question{entry, {2, -3}, -1000, 64, 1000, 2};
        const auto answer = pipeline.query(question);
        require(answer.finite == finite_scan(original, entry), "finite source oracle");
        close(answer.floating, float_scan(original, question.floating_entry));
        require(answer.source.source_id == 1001 && answer.source.version == 2 && answer.refinement_requested,
                "effect result drives source refinement and retains provenance");
        const std::set<u64> expected = strand == Strand::forward ? std::set<u64>{531, 532} : std::set<u64>{500, 501};
        require(support_coordinates(answer.refinement.selected) == expected && answer.refinement.deferred.empty(), "exact refined support oracle");
    }
    const auto blocked = pipeline.query({0, {2, -3}, -1000, 0, 1000, 2});
    require(blocked.refinement.selected.empty() && !blocked.refinement.deferred.empty(), "wave capacity produces explicit deferred support");
    const auto budgeted = pipeline.query({0, {2, -3}, -1000, 64, 0, 2});
    require(budgeted.refinement.examined == 0 && budgeted.refinement.deferred.size() == 1, "work budget deferral");
    const auto partial = pipeline.resume(blocked.refinement.deferred, 1);
    require(partial.produced == 2 && partial.stored == 1 && partial.dropped == 1 && partial.deferred.size() == 1,
            "deferred exact source replay capacity accounting");
    const auto completed = pipeline.resume(partial.deferred, 1);
    auto coordinates = support_coordinates(partial.selected);
    const auto remaining = support_coordinates(completed.selected);
    coordinates.insert(remaining.begin(), remaining.end());
    const std::set<u64> expected = strand == Strand::forward ? std::set<u64>{531, 532} : std::set<u64>{500, 501};
    require(coordinates == expected && completed.deferred.empty(), "resumed refinement reproduces unlimited result");
    require(pipeline.resume(blocked.refinement.deferred, 0).dropped == 2, "zero output capacity preserves deferred support");
    auto foreign = blocked.refinement.deferred; ++foreign.front().version;
    rejects<std::invalid_argument>([&] { pipeline.resume(foreign, 1); }, "foreign replay source version rejected");
    rejects<std::invalid_argument>([&] { pipeline.query({0, {2, -3}, 0, 1, 100, 3}); }, "stale query version rejected");
    rejects<std::invalid_argument>([&] { pipeline.query({32, {2, -3}, 0, 1, 100, 2}); }, "finite entry overflow rejected");
}
static void connected_policy() {
    const SequenceEffects pipeline(PackedFixture("G"), {1002, 9, 100, 1, Strand::forward, 1});
    const auto finite_low = pipeline.query({0, {2, -3}, 3, 4, 20, 1});
    const auto finite_high = pipeline.query({2, {2, -3}, 3, 4, 20, 1});
    require(finite_low.finite == 2 && finite_high.finite == 10 && finite_low.refinement_requested && !finite_high.refinement_requested,
            "incoming finite state affects concrete demand decision");
    const auto floating_low = pipeline.query({0, {0, 0}, 3, 4, 20, 1});
    require(!floating_low.refinement_requested && floating_low.refinement.selected.empty(), "incoming floating state affects demand decision");
    close(finite_low.floating, response::State{5.5, -4.625});
    const SequenceEffects empty(PackedFixture(""), {1003, 9, 100, 1, Strand::forward, 0});
    const auto identity = empty.query({7, {2, -3}, -100, 1, 10, 1});
    require(identity.finite == 7 && identity.refinement.selected.empty() && identity.refinement.deferred.empty(), "empty identity pipeline");
    close(identity.floating, response::State{2, -3});
    rejects<std::invalid_argument>([] { SequenceEffects bad(PackedFixture("C"), {1, 1, 0, 0, Strand::forward, 2}); }, "source length rejected");
}
int main() {
    try { full_connected_path(Strand::forward); full_connected_path(Strand::reverse); connected_policy();
        std::cout << "C01 predicates -> finite/affine trees -> incoming-state decision -> bounded source refinement/resume: PASS\n";
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
