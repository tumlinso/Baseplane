#include "prepared_sequence.hh"
#include <iostream>
#include <map>
using namespace sequence_tool;
static void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
template<class F> static void rejects(F call) {
    bool rejected = false;
    try { call(); } catch (const std::exception&) { rejected = true; }
    require(rejected, "expected contract rejection");
}
static reuse::source_snapshot source(std::string text, std::uint32_t halo = 0) {
    const auto n = static_cast<std::uint32_t>(text.size());
    return {std::move(text), {{17, 4, 2}, 100, n, halo, n - halo, halo, halo}, 3, 5};
}
static rep::vocabulary operations() {
    rep::vocabulary result;
    for (unsigned i = 0; i < 5; ++i) result[i] = rep::effect::identity();
    // A adds one, C doubles: AC and CA have equal counts and different response.
    result[0].b = {1, 0}; result[1].d = {2, 1};
    result[2].p = {1, 0}; result[3].b = {0, 1};
    return result;
}
static learn::route_model model() {
    std::array<bool, 8> observed{}; observed.fill(true);
    return learn::fit(observed, 11, 12, 13, 14, 200);
}
static int code(char c) {
    switch(c) { case 'A': return 0; case 'C': return 1; case 'G': return 2; case 'T': return 3; default: return -1; }
}
static rep::state scalar(const std::string& text, unsigned begin, unsigned end, rep::state state) {
    for (auto p = begin; p < end; ++p) {
        if (text[p] == 'A') state[0] += 1;
        if (text[p] == 'C') state[0] *= 2;
        if (text[p] == 'G') std::swap(state[0], state[1]);
        if (text[p] == 'T') state[1] += 1;
    }
    return state;
}
static void compare(const prepared_sequence& tool, const std::string& text, unsigned halo = 0) {
    std::map<std::string, unsigned> groups;
    for (unsigned p = halo; p < text.size() - halo && p + 2 <= text.size(); ++p)
        if (code(text[p]) >= 0 && code(text[p + 1]) >= 0) ++groups[text.substr(p, 2)];
    std::uint64_t pairs = 0;
    for (const auto& group : groups) pairs += group.second * (group.second - 1);
    for (const auto capacity : {0u, 1u, 3u, 200u}) for (unsigned alternative = 0; alternative < 2; ++alternative) {
        std::vector<rep::detail> details(capacity + 1); details.back().coordinate = 99999;
        std::vector<idx::pair> output(capacity + 1); output.back().first = 99999;
        prepared_sequence::request request{tool.question(halo, text.size() - 2 * halo), {2, 3}, alternative,
            learn::route_kind::hardened, true};
        const auto result = tool.execute(request, capacity ? details.data() : nullptr, capacity,
            capacity ? output.data() : nullptr, capacity);
        std::uint64_t valid = 0, gc = 0;
        for (unsigned p = halo; p < text.size() - halo; ++p) {
            const auto base = code(text[p]);
            if (base < 0) continue;
            if (valid < capacity) require(details[valid].coordinate == 100 + p
                && details[valid].logical == p && details[valid].code == base && details[valid].owned,
                "exact revisit source oracle");
            require(result.carriers[valid] == (base == 1 || base == 2 ? 1 : -1), "exact carrier oracle");
            ++valid; gc += base == 1 || base == 2;
        }
        require(std::get<query::exact_count>(result.valid_count.payload).value == valid
            && query::answer_matches(tool.view(), request.question, result.valid_count), "question exact answer");
        require(result.gc_matches == gc && result.summary.value.counts[1] + result.summary.value.counts[2] == gc,
            "native predicate/summary oracle");
        require(result.response == scalar(text, halo, text.size() - halo, {2, 3}), "ordered effect scalar oracle");
        require(seq::sequence_emit_counts_valid(seq::sequence_output_mode::stable_emit, capacity, result.detail.counts)
            && result.detail.counts.required_capacity == valid
            && result.detail.counts.stored_records == std::min<std::uint64_t>(capacity, valid), "detail capacity");
        require(result.nonlocal_pairs.required == pairs
            && result.nonlocal_pairs.stored == std::min<std::uint64_t>(capacity, pairs)
            && result.nonlocal_pairs.dropped == pairs - result.nonlocal_pairs.stored, "complete nonlocal group");
        for (std::uint64_t i = 0; i < result.nonlocal_pairs.stored; ++i) {
            const auto& first = tool.nonlocal_object(output[i].first);
            const auto& second = tool.nonlocal_object(output[i].second);
            require(first.id != second.id && first.exact == second.exact
                && first.support.source.identity.genome == 17
                && first.support.source.identity.contig == 4
                && first.support.source.value_generation == tool.view().value_generation,
                "nonlocal pair exact support identity");
        }
        require(details.back().coordinate == 99999 && output.back().first == 99999, "output sentinel");
        require(result.route.kind == learn::route_kind::hardened
            && result.route.gradient == learn::gradient_convention::stop_gradient
            && result.route.model.hardened == 13 && result.route.value_generation == tool.view().value_generation,
            "learned route identity/numerical policy");
        for (const auto& gate : result.route.gates) {
            const auto p = gate.anchor;
            require(p >= halo && p < text.size() - halo && gate.coordinate == 100 + p
                && gate.selected == (((text[p] == 'C' || text[p] == 'G') && text[p-1] == 'A') || text[p+1] == 'T'),
                "source-conditioned learned task oracle");
        }
    }
}
int main() {
    const auto fitted = model();
    prepared_sequence ac(source("AC"), operations(), fitted), ca(source("CA"), operations(), fitted);
    auto a = ac.execute({ac.question(0, 2)}, nullptr, 0, nullptr, 0);
    auto b = ca.execute({ca.question(0, 2)}, nullptr, 0, nullptr, 0);
    require(a.summary.value.counts == b.summary.value.counts && a.response != b.response, "actual order sensitivity");
    const std::string text = "AACGTNACACACAT";
    prepared_sequence tool(source(text), operations(), fitted);
    require(tool.predicate().semantic_hash != 0 && tool.predicate().live_node_count == 1, "prepared predicate");
    compare(tool, text);
    std::vector<idx::pair> complete_pairs(200);
    const auto group = tool.execute({tool.question(0, text.size())}, nullptr, 0, complete_pairs.data(), 200);
    bool distant = false;
    for (std::uint64_t i = 0; i < group.nonlocal_pairs.stored; ++i) {
        const auto x = tool.nonlocal_object(complete_pairs[i].first).support.first;
        const auto y = tool.nonlocal_object(complete_pairs[i].second).support.first;
        distant |= (x > y ? x - y : y - x) >= 4;
    }
    require(distant, "actual nonlocal group crossing separated sequence positions");
    prepared_sequence halo(source(text, 1), operations(), fitted); compare(halo, text, 1);
    auto stale = tool.question(0, text.size()); ++stale.expected_source.value_generation;
    rep::detail sentinel{99999, 0, 0, false};
    rejects([&] { tool.execute({stale}, &sentinel, 1, nullptr, 0); });
    require(sentinel.coordinate == 99999, "stale request changed output");
    auto wrong = tool.question(0, text.size()); ++wrong.expected_source.identity.contig;
    rejects([&] { tool.execute({wrong}, nullptr, 0, nullptr, 0); });
    wrong = tool.question(0, text.size()); wrong.strand = seq::source_strand::reverse;
    rejects([&] { tool.execute({wrong}, nullptr, 0, nullptr, 0); });
    rejects([&] { tool.execute({tool.question(0, 1)}, nullptr, 1, nullptr, 0); });
    auto lossy = ac.execute({ac.question(0, 2), {0, 0}, 0, learn::route_kind::soft_teacher, false}, nullptr, 0, nullptr, 0);
    require(lossy.carriers_lossy && lossy.carriers != a.carriers
        && lossy.route.kind == learn::route_kind::soft_teacher, "approximation explicit");
    const auto soft = tool.execute({tool.question(0, text.size()), {0, 0}, 0, learn::route_kind::soft_teacher}, nullptr, 0, nullptr, 0);
    require(soft.route.gradient == learn::gradient_convention::ce_bce_logits, "soft teacher convention");
    require(!soft.route.gates.empty() && soft.route.gates[0].probability > 0 && soft.route.gates[0].probability < 1,
        "real fitted probability");
    reuse::invalidation changed{};
    auto edited = tool.edited(tool.incremental().source().stamp(), {5, 1, "C"}, &changed);
    compare(edited, "AACGTCACACACAT"); compare(tool, text);
    const reuse::sequence_state rebuilt(edited.incremental().source(), 3, 2);
    require(edited.incremental().values() == rebuilt.values() && edited.incremental().directory() == rebuilt.directory()
        && !changed.rebuilt && !changed.memberships.empty() && changed.values.size() == 1
        && changed.after.value_generation == 6 && changed.after.structure_epoch == 3, "actual incremental repair");
    rejects([&] { edited.execute({tool.question(0, text.size())}, nullptr, 0, nullptr, 0); });
    auto resized = edited.edited(edited.incremental().source().stamp(), {3, 0, "AC"}, &changed);
    compare(resized, "AACACGTCACACACAT");
    require(changed.rebuilt && changed.after.structure_epoch == 4 && changed.after.value_generation == 7, "resize epoch");
    prepared_sequence empty(source(""), operations(), fitted); compare(empty, "");
    prepared_sequence invalid(source("NNNN"), operations(), fitted); compare(invalid, "NNNN");
    std::cout << "PASS order-sensitive effects; native GC predicate; exact revisit; complete nonlocal pairs; fitted routes; edit repair; capacity/identity/approximation contracts\n";
}
