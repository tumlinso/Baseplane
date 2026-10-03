#include <Baseplane/incremental/reuse.hh>
#include <iostream>

namespace reuse = baseplane::incremental;
static void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
template<class F> static void rejects(F call) {
    bool rejected = false;
    try { call(); } catch (const std::exception&) { rejected = true; }
    require(rejected, "expected rejection");
}
static reuse::source_snapshot source(std::string text, reuse::strand orientation = reuse::strand::forward) {
    const auto size = static_cast<std::uint32_t>(text.size());
    return {std::move(text), {{17, 4, 2}, 100, size, 0, size, 0, 0}, 3, 5, orientation};
}
static reuse::guard_key key(const reuse::source_snapshot& source, reuse::support support = {0, 3}) {
    return {source.stamp(), 9, 2, 3, 4, 5, 6, 0, 1, 7, support};
}
static reuse::u64 scalar_gc(const std::string& text, reuse::support support) {
    reuse::u64 result = 0;
    for (auto p = support.begin; p < support.end; ++p)
        result += text[p] == 'C' || text[p] == 'G' || text[p] == 'c' || text[p] == 'g';
    return result;
}
static void check_full_rebuild(const reuse::sequence_state& state, std::uint32_t chunk, std::uint32_t window) {
    const reuse::sequence_state oracle(state.source(), chunk, window);
    require(state.values() == oracle.values() && state.directory() == oracle.directory(), "incremental/full rebuild");
    std::vector<reuse::u64> values;
    for (std::uint32_t begin = 0; begin < state.source().exact.size(); begin += chunk)
        values.push_back(scalar_gc(state.source().exact,
            {begin, static_cast<std::uint32_t>(std::min<std::size_t>(state.source().exact.size(), begin + chunk))}));
    require(state.values() == values, "independent scalar values");
    std::map<std::string, std::set<std::size_t>> directory;
    const auto& text = state.source().exact;
    for (std::size_t p = 0; p + window <= text.size(); ++p) {
        std::string exact; bool valid = true;
        for (std::size_t j = 0; j < window; ++j) {
            const auto token = text[p + j];
            valid &= token == 'A' || token == 'C' || token == 'G' || token == 'T';
            exact += token;
        }
        if (valid) directory[exact].insert(p);
    }
    require(state.directory() == directory, "independent directory membership");
}
static void seam_edit_and_resize() {
    const reuse::sequence_state base(source("AACCGGTTNACG"), 4, 3);
    reuse::invalidation dirty{};
    const auto changed = base.edited(base.source().stamp(), {3, 2, "TA"}, &dirty);
    require(dirty.values == std::vector<std::size_t>({0, 1})
        && dirty.memberships == std::vector<std::size_t>({1, 2, 3, 4}), "seam dirty cone");
    require(changed.source().exact == "AACTAGTTNACG" && base.source().exact == "AACCGGTTNACG",
        "immutable predecessor snapshot");
    require(dirty.before.value_generation == 5 && dirty.after.value_generation == 6
        && dirty.before.structure_epoch == dirty.after.structure_epoch && !dirty.rebuilt, "value/structure separation");
    require(changed.directory().count("ACC") == 0 && changed.directory().count("ACT") == 1
        && changed.directory().count("TAG") == 1, "seam old memberships replaced");
    check_full_rebuild(changed, 4, 3);
    for (std::uint32_t position = 0; position < base.source().exact.size(); ++position) {
        const auto edited = base.edited(base.source().stamp(), {position, 1, position % 2 ? "N" : "G"}, &dirty);
        check_full_rebuild(edited, 4, 3);
    }
    const auto inserted = base.edited(base.source().stamp(), {4, 0, "AC"}, &dirty);
    require(dirty.rebuilt && dirty.after.structure_epoch == 4 && inserted.source().exact == "AACCACGGTTNACG",
        "resize structural invalidation");
    check_full_rebuild(inserted, 4, 3);
    const auto removed = base.edited(base.source().stamp(), {2, 5, ""}, &dirty);
    check_full_rebuild(removed, 4, 3);
    auto stale = base.source().stamp(); ++stale.value_generation;
    rejects([&] { base.edited(stale, {0, 1, "C"}, &dirty); });
    rejects([&] { base.edited(base.source().stamp(), {100, 1, "C"}, &dirty); });
    const reuse::sequence_state empty(source(""), 4, 3);
    const auto populated = empty.edited(empty.source().stamp(), {0, 0, "ACGT"}, &dirty);
    check_full_rebuild(populated, 4, 3);
}
static void sharing_and_guards() {
    reuse::content_pool pool;
    const auto content = pool.intern("AACG", 42);
    const auto same = pool.intern("AACG", 42);
    require(content == same && content->gc == 2, "context-free exact work not shared");
    require(pool.intern("AANG", 42) != pool.intern("AA-G", 42), "invalid original tokens conflated");
    require(pool.created() == 3, "nomination collision merged different contents");
    auto a = key(source("AACG")); auto b = a; ++b.occurrence; ++b.context_generation;
    reuse::occurrence first(content, a, .2), second(content, b, -.4);
    require(first.shared_content == second.shared_content && !(first.guard == second.guard)
        && first.contextual_residual != second.contextual_residual, "occurrence/context aliased");
    reuse::result_store<reuse::u64> results; reuse::plan_store<reuse::support> plans;
    reuse::residual_store<std::vector<double>> residuals;
    results.put(a, 2); plans.put(a, {0, 3}); residuals.put(a, {.25, -.5});
    require(results.find(a) && plans.find(a) && residuals.find(a), "typed guarded stores");
    const std::vector<reuse::guard_key> changed = [&] {
        std::vector<reuse::guard_key> output;
        auto mutate = [&](auto fn) { auto next = a; fn(next); output.push_back(next); };
        mutate([](auto& k) { ++k.source.identity.genome; }); mutate([](auto& k) { ++k.source.identity.contig; });
        mutate([](auto& k) { ++k.source.identity.chunk; }); mutate([](auto& k) { ++k.source.structure_epoch; });
        mutate([](auto& k) { ++k.source.value_generation; }); mutate([](auto& k) { ++k.source.origin; });
        mutate([](auto& k) { ++k.source.length; }); mutate([](auto& k) { k.source.orientation = reuse::strand::reverse; });
        mutate([](auto& k) { ++k.occurrence; }); mutate([](auto& k) { ++k.context_generation; });
        mutate([](auto& k) { ++k.model; }); mutate([](auto& k) { ++k.weights_generation; });
        mutate([](auto& k) { ++k.representation_generation; }); mutate([](auto& k) { ++k.numerical_policy; });
        mutate([](auto& k) { ++k.world; }); mutate([](auto& k) { ++k.world_generation; });
        mutate([](auto& k) { ++k.query; }); mutate([](auto& k) { ++k.inspected.begin; });
        mutate([](auto& k) { ++k.inspected.end; }); return output;
    }();
    for (const auto& changed : changed)
        require(!results.find(changed) && !plans.find(changed) && !residuals.find(changed), "incomplete artifact guard key");
    auto malformed = a; malformed.inspected.end = 100;
    rejects([&] { results.put(malformed, 1); });
}
static void memos_and_worlds() {
    const reuse::sequence_state base(source("AACGTA"), 3, 2);
    const auto saved = key(base.source());
    const auto answer = scalar_gc(base.source().exact, saved.inspected);
    reuse::invalidation dirty{};
    const auto outside = base.edited(base.source().stamp(), {5, 1, "C"}, &dirty);
    auto current = saved; current.source = outside.source().stamp();
    require(reuse::reusable(saved, current, {dirty.edit})
        && answer == scalar_gc(outside.source().exact, saved.inspected), "outside-support guarded reuse");
    require(!reuse::reusable(saved, current, {}), "missing edit chain accepted");
    const auto inside = base.edited(base.source().stamp(), {2, 1, "N"}, &dirty);
    current.source = inside.source().stamp();
    require(!reuse::reusable(saved, current, {dirty.edit}), "negative/invalid support evidence lost");
    auto changed_weights = saved; ++changed_weights.weights_generation;
    require(!reuse::reusable(saved, changed_weights, {}), "weights guard");
    auto wrong_record = dirty.edit; ++wrong_record.from_generation;
    require(!reuse::reusable(saved, current, {wrong_record}), "unordered edit chain");
    const auto resized = base.edited(base.source().stamp(), {5, 0, "A"}, &dirty);
    current.source = resized.source().stamp();
    require(!reuse::reusable(saved, current, {dirty.edit}), "resize must invalidate coordinate-bearing result");
    reuse::world_delta branch{saved, 91, 1, {{2, 1, "A"}}};
    const auto world = reuse::derive_world(base, saved, branch);
    require(world.state.source().exact == "AAAGTA" && world.guard.world == 91
        && world.baseline == saved && world.invalidated.size() == 1
        && world.guard.source.value_generation == 6, "world delta baseline lineage");
    check_full_rebuild(world.state, 3, 2);
    const auto equal_world = reuse::derive_world(base, saved, {saved, 92, 1, {}});
    require(equal_world.state.source().exact == base.source().exact
        && !(equal_world.guard == saved), "identical counterfactual world alias");
    reuse::result_store<reuse::u64> results;
    results.put(saved, answer);
    require(!results.find(world.guard) && !results.find(equal_world.guard), "world result guards");
    auto stale = branch; ++stale.baseline.context_generation;
    rejects([&] { reuse::derive_world(base, saved, stale); });
    stale = branch; ++stale.baseline.world_generation;
    rejects([&] { reuse::derive_world(base, saved, stale); });
    const reuse::sequence_state reverse(source("AACGTA", reuse::strand::reverse), 3, 2);
    const auto reverse_key = key(reverse.source());
    const auto reverse_edited = reverse.edited(reverse.source().stamp(), {5, 0, "A"}, &dirty);
    auto reverse_current = reverse_key; reverse_current.source = reverse_edited.source().stamp();
    require(!reuse::reusable(reverse_key, reverse_current, {dirty.edit}), "reverse resize coordinates");
}
int main() {
    seam_edit_and_resize(); sharing_and_guards(); memos_and_worlds();
    std::cout << "seam edit values/postings, content/occurrences, full guards and worlds passed\n";
}
