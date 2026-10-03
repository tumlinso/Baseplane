#include <Baseplane/representation/hierarchy.hh>
#include <iostream>
#include <string>

namespace rep = baseplane::representation;
namespace seq = baseplane::seq;
static void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}
template<class F> static void rejects(F call) {
    bool rejected = false;
    try { call(); } catch (const std::exception&) { rejected = true; }
    require(rejected, "expected rejection");
}
static int code(char c) {
    switch (c) { case 'A': return 0; case 'C': return 1; case 'G': return 2; case 'T': return 3; default: return -1; }
}
static rep::source_owner snapshot(const std::string& text, rep::strand strand = rep::strand::forward) {
    std::vector<std::uint64_t> packed(seq::dna2_packed64_word_count(text.size()), 0);
    std::vector<std::uint64_t> valid(seq::dna2_validity64_word_count(text.size()), 0);
    for (std::size_t p = 0; p < text.size(); ++p) {
        const auto encoded = seq::dna2_encode_base_with_validity(text[p]);
        packed[p / 32] |= std::uint64_t{encoded.valid ? encoded.code : std::uint8_t{3}} << (2 * (p % 32));
        if (encoded.valid) valid[p / 64] |= std::uint64_t{1} << (p % 64);
    }
    const auto n = static_cast<std::uint32_t>(text.size()), halo = n > 2 ? 1u : 0u;
    return std::make_shared<const rep::exact_source>(
        seq::dna2_packed64_valid_view{{packed.data(), n, packed.size()}, valid.data(), valid.size()},
        seq::dna2_chunk_coordinates{{7, 9, 2}, 100, n, halo, n - halo, halo, halo}, 3, 5, strand);
}
static rep::vocabulary operations() {
    rep::vocabulary result;
    for (int base = 0; base < 4; ++base) {
        auto operation = rep::effect::identity();
        if (base == 1) operation.p = {1, 0};
        operation.d = {1 + .125 * base, 1 - .0625 * base};
        operation.b = {double(base + 1), double(-base)};
        result[base] = operation;
    }
    result[4] = rep::effect::identity(); result[4].d.fill(0);
    return result;
}
static void near(rep::state a, rep::state b) {
    for (std::size_t i = 0; i < 2; ++i)
        require(std::abs(a[i] - b[i]) <= 1e-10 * std::max(1., std::abs(b[i])), "ordered numerical oracle");
}
static int logical_code(const std::string& text, std::uint32_t logical, rep::strand strand) {
    const auto physical = strand == rep::strand::forward ? logical : text.size() - 1 - logical;
    const auto base = code(text[physical]);
    return base < 0 || strand == rep::strand::forward ? base : base ^ 3;
}
static void check(const std::string& text, rep::strand strand) {
    const auto source = snapshot(text, strand);
    const auto cuts = rep::choose_cuts(*source, 2, 5, .5,
        [](int left, int right, std::uint32_t) { return left != right ? 1. : 0.; });
    rep::hierarchy hierarchy(source, operations(), {{}, cuts});
    require(hierarchy.source() == source && hierarchy.forests() == 2, "shared source ownership");
    require(hierarchy.inspected_bases() == 3 * text.size(), "construction source cost omitted");
    for (std::uint32_t begin = 0; begin <= text.size(); ++begin) {
        const auto end = std::min<std::uint32_t>(text.size(), begin + 7);
        rep::summary expected;
        rep::state entry{2, -3}, state = entry;
        for (auto p = begin; p < end; ++p) {
            const auto base = logical_code(text, p, strand);
            if (base < 0) { ++expected.invalid; state = {}; continue; }
            ++expected.counts[base];
            const auto old = state;
            state[0] = (1 + .125 * base) * old[base == 1 ? 1 : 0] + base + 1;
            state[1] = (1 - .0625 * base) * old[base == 1 ? 0 : 1] - base;
        }
        for (std::size_t forest = 0; forest < hierarchy.forests(); ++forest) {
            const auto answer = hierarchy.query(source->key(), {begin, end}, forest);
            require(answer.value.counts == expected.counts && answer.value.invalid == expected.invalid,
                "histogram portfolio oracle");
            near(hierarchy.response(source->key(), {begin, end}, entry, forest), state);
            require(answer.replayed_bases <= end - begin, "revisit cost");
        }
    }
    std::vector<double> carriers;
    std::vector<std::uint32_t> positions;
    for (std::uint32_t p = 0; p < text.size(); ++p) {
        const auto base = logical_code(text, p, strand);
        if (base < 0) continue;
        carriers.push_back(base == 1 || base == 2 ? 1. : -1.);
        positions.push_back(p);
    }
    require(hierarchy.carriers(true) == carriers && hierarchy.carrier_positions() == positions,
        "full residual reconstruction oracle");
    require(hierarchy.residual_bytes() == carriers.size() / 2 * sizeof(double), "residual accounting");
    std::vector<rep::detail> output(2);
    for (bool include_halos : {false, true}) {
        const auto answer = hierarchy.revisit(source->key(), {0, source->size()}, output.data(), 1, include_halos);
        std::uint64_t matches = 0, invalid = 0, halos = 0;
        for (std::uint32_t p = 0; p < text.size(); ++p) {
            const auto physical = strand == rep::strand::forward ? p : text.size() - 1 - p;
            const auto base = logical_code(text, p, strand);
            if (base < 0) { ++invalid; continue; }
            const bool owned = text.size() <= 2 || (physical > 0 && physical + 1 < text.size());
            if (!include_halos && !owned) { ++halos; continue; }
            if (!matches) {
                require(output[0].coordinate == 100 + physical && output[0].logical == p
                    && output[0].code == base && output[0].owned == owned, "source detail oracle");
            }
            ++matches;
        }
        require(answer.source == source->key() && answer.examined == text.size()
            && answer.invalid == invalid && answer.excluded_halos == halos, "detail provenance/accounting");
        require(answer.counts.total_matches == matches
            && seq::sequence_emit_counts_valid(seq::sequence_output_mode::stable_emit, 1, answer.counts),
            "detail output capacity");
        const auto count = hierarchy.revisit(source->key(), {0, source->size()}, nullptr, 0, include_halos);
        require(count.counts.stored_records == 0 && count.counts.required_capacity == matches,
            "zero capacity exact replay");
    }
}
int main() {
    for (const auto& text : {std::string{}, std::string{"AN"}, std::string{"GAGANCGTACAGT"},
                           std::string(33, 'C') + "NAGCT", std::string(65, 'A')})
        for (const auto strand : {rep::strand::forward, rep::strand::reverse}) check(text, strand);
    const auto source = snapshot("GAGAG");
    rep::hierarchy hierarchy(source, operations(), {{2}, {1, 4}});
    require(hierarchy.carriers(false) != hierarchy.carriers(true), "coarse omission hidden");
    require(hierarchy.query(source->key(), {1, 4}, 1).replayed_bases == 0
        && hierarchy.query(source->key(), {1, 4}, 0).replayed_bases > 0, "alternate boundaries not usable");
    auto stale = source->key(); ++stale.value_generation;
    rejects([&] { hierarchy.query(stale, {0, 1}); });
    rejects([&] { hierarchy.revisit(stale, {0, 1}, nullptr, 0); });
    rejects([&] { hierarchy.query(source->key(), {3, 2}); });
    rejects([&] { hierarchy.query(source->key(), {0, 6}); });
    rejects([&] { hierarchy.query(source->key(), {0, 1}, 2); });
    rejects([&] { hierarchy.revisit(source->key(), {0, 1}, nullptr, 1); });
    rejects([&] { rep::hierarchy bad(source, operations(), {{2, 2}}); });
    rejects([&] { rep::hierarchy bad(source, operations(), {}); });
    rejects([&] { rep::choose_cuts(*source, 0, 4, .5, [](int, int, std::uint32_t){ return 0.; }); });
    rejects([&] { rep::choose_cuts(*source, 1, 4, .5, [](int, int, std::uint32_t){
        return std::numeric_limits<double>::quiet_NaN(); }); });
    auto bad_operations = operations(); bad_operations[0].p = {0, 0};
    rejects([&] { rep::hierarchy bad(source, bad_operations, {{}}); });
    rep::hierarchy ac(snapshot("AC"), operations(), {{}});
    rep::hierarchy ca(snapshot("CA"), operations(), {{}});
    require(ac.response(ac.source()->key(), {0, 2}, {2, -3})
        != ca.response(ca.source()->key(), {0, 2}, {2, -3}), "composition order lost");
    std::cout << "hierarchy CE effects/lifting and source consumers passed\n";
}
