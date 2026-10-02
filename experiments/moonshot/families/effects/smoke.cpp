#include "response.hpp"
#include <iostream>

using namespace bp_moon;
using namespace bp_moon::response;
static void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
template<std::size_t N> static void close(const std::array<double, N>& a, const std::array<double, N>& b, const char* message) {
    for (std::size_t i = 0; i < N; ++i) require(std::abs(a[i] - b[i]) < 1e-10 * std::max(1., std::abs(b[i])), message);
}
template<class E, class F> static void rejects(F fn, const char* message) {
    try { fn(); } catch (const E&) { return; }
    throw std::runtime_error(message);
}
// Independent scalar sequence oracle, restricted to this source vocabulary.
static State scan(std::string_view sequence, State input) {
    for (char base : sequence) {
        const auto code = base_code(base);
        if (code < 0) { input = {}; continue; }
        const auto old = input;
        input[0] = (1 + code * .125) * old[code == 1 ? 1 : 0] + code + 1;
        input[1] = (1 - code * .0625) * old[code == 1 ? 0 : 1] - code;
    }
    return input;
}
static BlockState dense_scan(std::string_view sequence, BlockState input) {
    for (char base : sequence) {
        const auto effect = block_base(base);
        std::array<std::array<double, 4>, 4> dense{};
        BlockState bias{};
        for (std::size_t block = 0; block < 2; ++block) for (std::size_t row = 0; row < 2; ++row) {
            bias[block * 2 + row] = effect.bias[block][row];
            for (std::size_t col = 0; col < 2; ++col)
                dense[block * 2 + row][effect.p[block] * 2 + col] = effect.matrix[block][row][col];
        }
        BlockState next = bias;
        for (std::size_t i = 0; i < 4; ++i) for (std::size_t j = 0; j < 4; ++j) next[i] += dense[i][j] * input[j];
        input = next;
    }
    return input;
}
static void e09() {
    const PackedFixture sequence("AcGTNA");
    const SourceMap map{81, 4, 1000, 7, Strand::reverse, sequence.original.size()};
    const ResponseHierarchy hierarchy(sequence, map);
    for (const State input : {State{0, 0}, State{2, -3}, State{-1, 4}})
        close(ce_moon::apply(hierarchy.nodes[hierarchy.root].effect, input), scan(sequence.original, input), "E09 hierarchy direct oracle");
    require(hierarchy.source.source_id == 81 && hierarchy.source.coordinate(0) == 1005, "E09 retained provenance");
    const auto left = summarize_monomial("AC"), right = summarize_monomial("G");
    const State input{2, -3};
    close(ce_moon::apply(ce_moon::compose(left, right), input), ce_moon::apply(right, ce_moon::apply(left, input)), "E09 seam sequential");
    require(ce_moon::apply(summarize_monomial("AC"), input) != ce_moon::apply(summarize_monomial("CA"), input), "E09 order sensitivity");
    close(ce_moon::apply(summarize_monomial("N"), input), State{}, "E09 invalid resets");
    close(ce_moon::apply(summarize_monomial(""), input), input, "E09 empty identity");
}
static void e10() {
    const BlockState input{1, 2, -1, 3};
    const auto composed = ce_moon::effects::compose(ce_moon::effects::compose(block_base('A'), block_base('C')), block_base('G'));
    close(ce_moon::effects::apply(composed, input), dense_scan("ACG", input), "E10 dense expansion three blocks");
    close(ce_moon::effects::apply(summarize_blocks("ANcGT"), input), dense_scan("ANcGT", input), "E10 validity and sequence order");
    const PackedFixture sequence("ACG");
    const BlockRegion region(sequence, SourceMap{84, 5, 200, 6, Strand::forward, 3});
    close(ce_moon::effects::apply(region.effect, input), dense_scan(sequence.original, input), "E10 source region response");
    require(region.source.contig == 5 && region.source.version == 6, "E10 source provenance");
    const auto identity = summarize_blocks("");
    close(ce_moon::effects::apply(identity, input), input, "E10 empty identity");
    require(block_base('A').matrix[0][0][1] != 0 && block_base('A').matrix[0][0][0] != 0,
            "E10 genuine within-block mixing beyond monomial");
}
static void e11() {
    const PackedFixture sequence("AC");
    const SourceMap source{82, 1, 50, 3, Strand::forward, 2};
    JetRegion region(sequence, source, 0, .25);
    const auto local = region.query(.1);
    const auto exact_local = exact_response(sequence.original, .1);
    require(!local.refined && !local.certified_error_bound && local.source_bases_replayed == 0, "E11 in-radius approximation");
    const auto error = exact_local - local.value;
    require(std::abs(error - .0000402) < 1e-12, "E11 higher-order loss measured");
    const auto far = region.query(1);
    require(far.refined && far.source_bases_replayed == 2 && far.source_bases_reanchored == 2 && std::abs(far.value - 1.342) < 1e-12, "E11 out-radius source replay");
    require(region.jet().center == 1 && region.source().version == 3, "E11 reanchored expansion and provenance");
    require(!region.query(1).refined, "E11 cached refined center");
    require(exact_response("ANC", 2) == 0, "E11 unknown reset");
    rejects<std::invalid_argument>([&] { region.query(std::numeric_limits<double>::quiet_NaN()); }, "E11 nonfinite query rejected");
    std::cout << "E11 in-radius truncation error=" << error << "; out-radius replay=" << far.value << '\n';
}
static void e12() {
    const PackedFixture sequence("AcGTNACGT");
    const SourceMap source{83, 3, 800, 1, Strand::forward, sequence.original.size()};
    const std::size_t stride = 3;
    const SequenceCheckpoints checkpoints(sequence, source, stride);
    const State left{2, -1}, right{-3, 4};
    for (std::size_t boundary = 0; boundary <= source.length; ++boundary) {
        const auto result = checkpoints.query(boundary, left, right);
        const auto prefix = sequence.original.substr(0, boundary);
        auto reverse_suffix = sequence.original.substr(boundary);
        std::reverse(reverse_suffix.begin(), reverse_suffix.end());
        close(result.left, scan(prefix, left), "E12 left checkpoint reconstruction");
        close(result.right, scan(reverse_suffix, right), "E12 right reverse-order context");
        require(result.left_replayed < stride && result.right_replayed < stride, "E12 replay cost bounded by stride");
        std::size_t replayed = 0;
        close(checkpoints.forward_suffix(boundary, right, &replayed), scan(sequence.original.substr(boundary), right),
              "E12 forward suffix response distinct from reverse context");
        require(replayed < stride, "E12 suffix replay bound");
    }
    require(checkpoints.source().source_id == 83 && checkpoints.retained_maps() == 16, "E12 source and retained maps");
    rejects<std::out_of_range>([&] { checkpoints.query(10, left, right); }, "E12 boundary overflow rejected");
    rejects<std::invalid_argument>([&] { SequenceCheckpoints bad(sequence, source, 0); }, "E12 zero stride rejected");
}
int main() {
    try { e09(); e10(); e11(); e12(); std::cout << "E09-E12 source-grounded continuous effects: PASS\n"; }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
