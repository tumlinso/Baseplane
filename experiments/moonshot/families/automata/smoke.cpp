#include "automata.hpp"
#include <iostream>
#include <cstdlib>

using namespace bp_moon;
using namespace bp_moon::automata;
static void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
template<class Exception, class F> static void rejects(F fn, const char* message) {
    try { fn(); } catch (const Exception&) { return; }
    throw std::runtime_error(message);
}
static unsigned scan_state(std::string_view sequence, unsigned state) {
    for (char raw : sequence) {
        const auto base = canonical(raw);
        const auto code = std::string_view("ACGT").find(base);
        state = code == std::string_view::npos ? 0 : (state * 4 + code) % 16;
    }
    return state;
}
static void e05() {
    const PackedFixture source("AcGTNcaT");
    const EffectHierarchy tree(source);
    for (unsigned state = 0; state < 32; ++state)
        require(tree.nodes[tree.root].effect.to[state] == scan_state(source.original, state), "E05 direct state");
    const auto a = summarize_sequence("A"), c = summarize_sequence("C"), g = summarize_sequence("G");
    require(ce_moon::compose(a, c).to != ce_moon::compose(c, a).to, "E05 order sensitivity");
    require(ce_moon::compose(ce_moon::compose(a, c), g).to == ce_moon::compose(a, ce_moon::compose(c, g)).to,
            "E05 associativity");
    require(EffectHierarchy(PackedFixture("")).nodes[0].effect.to == ce_moon::Dfa32::identity().to, "E05 empty identity");
}
static void e06() {
    const auto left = summarize_motif("A"), right = summarize_motif("CAC");
    const auto joined = concatenate(left, right), whole = summarize_motif("ACAC");
    require(right.effect.count[0] == 1 && right.effect.count[1] == 2, "E06 entry-dependent count");
    for (unsigned state = 0; state < 32; ++state) {
        require(joined.effect.count[state] == whole.effect.count[state] && joined.first[state] == whole.first[state] &&
                joined.last[state] == whole.last[state], "E06 seam composition");
    }
    require(joined.first[0] == 1 && joined.last[0] == 3, "E06 witnesses");
    const PackedFixture source("ACACnAC");
    const SourceMap forward{71, 2, 100, 9, Strand::forward, source.original.size()};
    const SourceMap reverse{71, 2, 100, 9, Strand::reverse, source.original.size()};
    u64 out[3]{};
    auto count = replay_motif(source, forward, out, 1);
    require(count.produced == 3 && count.stored == 1 && count.dropped == 2 && out[0] == 101, "E06 bounded replay");
    count = replay_motif(source, reverse, out, 3);
    require(count.produced == 3 && out[0] == 105 && out[1] == 103 && out[2] == 100, "E06 strand coordinate mapping");
    require(summarize_motif("ANC").effect.count[0] == 0, "E06 validity reset");
    require(replay_motif(source, forward, nullptr, 0).dropped == 3, "E06 zero capacity");
    auto overflowing = left;
    overflowing.effect.count[0] = std::numeric_limits<u64>::max();
    rejects<std::overflow_error>([&] { concatenate(overflowing, right); }, "E06 checked count overflow");
}
static void e07() {
    const auto joined = concatenate(grammar_tile("AC", 4), grammar_tile("GT", 4), 4);
    require(grammar_accepts(joined) && joined.maximum == 2 && joined.net_depth == 0, "E07 cross-seam pair");
    require(!grammar_accepts(concatenate(grammar_tile("A", 4), grammar_tile("G", 4), 4)), "E07 mismatched seam");
    const auto overflow = grammar_tile("AAATTT", 2);
    require(overflow.needs_replay && grammar_replay("AAATTT"), "E07 overflow and exact replay");
    rejects<std::logic_error>([&] { grammar_accepts(overflow); }, "E07 overflow cannot silently accept");
    require(grammar_tile("AN", 4).needs_replay && !grammar_replay("AN"), "E07 unknown validity");
    // Exhaustive small synthetic source fixtures verify seam behavior independently.
    const std::string alphabet = "ACGTN";
    std::size_t fixtures = 1;
    for (unsigned length = 0; length <= 5; ++length) {
        if (length) fixtures *= alphabet.size();
        for (std::size_t key = 0; key < fixtures; ++key) {
            std::string source(length, 'A');
            auto digits = key;
            for (char& base : source) { base = alphabet[digits % alphabet.size()]; digits /= alphabet.size(); }
            for (unsigned seam = 0; seam <= length; ++seam) {
                const auto motif = concatenate(summarize_motif(std::string_view(source).substr(0, seam)),
                                               summarize_motif(std::string_view(source).substr(seam)));
                for (unsigned entry = 0; entry < 2; ++entry) {
                    bool prefix = entry == 1;
                    std::vector<std::size_t> events;
                    for (std::size_t i = 0; i < source.size(); ++i) {
                        if (prefix && source[i] == 'C') events.push_back(i);
                        prefix = source[i] == 'A';
                    }
                    require(motif.effect.count[entry] == events.size(), "E06 exhaustive count oracle");
                    if (events.empty()) require(!motif.first[entry] && !motif.last[entry], "E06 empty witnesses");
                    else require(motif.first[entry] == events.front() && motif.last[entry] == events.back(),
                                 "E06 exhaustive witness oracle");
                }
                const auto summary = concatenate(grammar_tile(std::string_view(source).substr(0, seam), 8),
                                                 grammar_tile(std::string_view(source).substr(seam), 8), 8);
                if (!summary.needs_replay) require(grammar_accepts(summary) == grammar_replay(source), "E07 exhaustive seam oracle");
            }
        }
    }
}
int main() {
    try {
        e05(); e06(); e07();
        std::cout << "E05 exact all-entry hierarchy; E06 counted witnessed source replay; E07 bounded grammar seams: PASS\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
