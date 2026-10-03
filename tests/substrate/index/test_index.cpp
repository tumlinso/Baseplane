#include <Baseplane/index/sequence_index.hh>
#include <iostream>

namespace idx = baseplane::index;
namespace seq = baseplane::seq;
static void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
template<class F> static void rejects(F call) {
    bool rejected = false;
    try { call(); } catch (const std::exception&) { rejected = true; }
    require(rejected, "expected rejection");
}
struct Fixture {
    std::string text;
    std::vector<idx::u64> packed, valid;
    explicit Fixture(std::string text) : text(std::move(text)),
        packed(seq::dna2_packed64_word_count(this->text.size()), 0),
        valid(seq::dna2_validity64_word_count(this->text.size()), 0) {
        for (std::size_t p = 0; p < this->text.size(); ++p) {
            const auto base = seq::dna2_encode_base_with_validity(this->text[p]);
            packed[p / 32] |= idx::u64{base.valid ? base.code : std::uint8_t{3}} << (2 * (p % 32));
            if (base.valid) valid[p / 64] |= idx::u64{1} << (p % 64);
        }
    }
    idx::source_input input(idx::u64 genome = 7, std::uint32_t role = 0,
                            idx::strand strand = idx::strand::forward, std::uint32_t halo = 0) const {
        const auto size = static_cast<std::uint32_t>(text.size());
        return {{{packed.data(), size, packed.size()}, valid.data(), valid.size()},
            {{genome, 9, 2}, 100, size, halo, size - halo, halo, halo}, 3, 5, strand, role};
    }
};
static int text_code(char base) {
    switch (base) { case 'A': return 0; case 'C': return 1; case 'G': return 2; case 'T': return 3; default: return -1; }
}
static void extraction_and_large_ranges() {
    const Fixture big(std::string(137, 'A'));
    idx::sequence_index index({big.input()}, 2);
    const auto range = index.equal_range("aa");
    require(range.second - range.first == 136, "range truncated at warp or row");
    for (std::size_t i = range.first; i < range.second; ++i)
        require(index.posting_at(i).support.first == 100 + i && index.posting_at(i).exact == "AA", "posting provenance");
    idx::pair pairs[3]{}; pairs[2] = {99999, 99999};
    const auto count = index.equal_pairs(pairs, 2);
    require(count.required == 136 * 135 && count.stored == 2 && count.dropped == count.required - 2,
        "oversized group complete pair accounting");
    require(pairs[0].first == 0 && pairs[0].second == 1 && pairs[2].first == 99999, "pair capacity");
    require(index.equal_pairs(nullptr, 0).required == count.required, "count only pair output");
    const auto nominations = index.nominate(2, 100000);
    require(nominations.counts.required == 136 * 135 / 2
        && nominations.candidates.size() == nominations.counts.required, "duplicate two-probe candidates");
    const auto recall = index.evaluate_recall(nominations, 0);
    require(recall.truth == nominations.counts.required && recall.missed == 0, "large global recall");
    rejects([&] { index.equal_pairs(nullptr, 1); });
    rejects([&] { index.equal_range("N"); });
    rejects([&] { index.equal_range("A"); });
    const Fixture whole_input(std::string(5003, 'A'));
    idx::sequence_index whole({whole_input.input()}, 2);
    require(whole.equal_range("AA").second == 5002, "old fixture limit retained");
    const auto complete_count = whole.nominate(2, 0);
    require(complete_count.counts.required == 5002ull * 5001 / 2
        && complete_count.counts.dropped == complete_count.counts.required, "whole-input nomination capacity");
    const Fixture text("ACGTNACGTACGTACGTACGTACGTACGTACGTNACGT");
    for (const auto orientation : {idx::strand::forward, idx::strand::reverse}) {
        idx::sequence_index local({text.input(17, 2, orientation, 1)}, 3);
        std::vector<std::string> expected;
        std::vector<std::pair<idx::u64, idx::u64>> coordinates;
        for (std::size_t p = 0; p + 3 <= text.text.size(); ++p) {
            const auto physical = [&](std::size_t logical) {
                return orientation == idx::strand::forward ? logical : text.text.size() - 1 - logical;
            };
            if (physical(p) == 0 || physical(p) + 1 == text.text.size()) continue;
            std::string key; bool valid = true;
            for (std::size_t j = 0; j < 3; ++j) {
                const auto base = text_code(text.text[physical(p + j)]);
                if (base < 0) { valid = false; continue; }
                key += "ACGT"[orientation == idx::strand::forward ? base : base ^ 3];
            }
            if (valid) { expected.push_back(key); coordinates.emplace_back(100 + physical(p), 100 + physical(p + 2)); }
        }
        require(local.objects().size() == expected.size(), "scalar extracted object count");
        for (std::size_t i = 0; i < expected.size(); ++i) {
            const auto& object = local.at(i);
            require(object.exact == expected[i] && object.support.first == coordinates[i].first
                && object.support.last == coordinates[i].second && object.support.source.role == 2
                && object.support.source.orientation == orientation && object.support.source.value_generation == 5,
                "invalid/tail/strand/source role oracle");
        }
    }
}
static void recall_verification_scoring() {
    const Fixture a("AAAA"), b("CAAA"), c("CAAC"), d("AATT");
    idx::sequence_index index({a.input(1), b.input(2), c.input(3), d.input(4)}, 4);
    const auto one = index.nominate(1, 100);
    const auto two = index.nominate(2, 100);
    const auto one_recall = index.evaluate_recall(one, 1);
    const auto two_recall = index.evaluate_recall(two, 1);
    require(one_recall.truth == 2 && one_recall.missed == 1
        && two_recall.truth == 2 && two_recall.missed == 0, "nomination recall must be separately measured");
    const auto exact = index.verify({0, 1}, 1);
    require(exact.accepted && exact.distance == 1 && !index.verify({0, 3}, 1).accepted, "exact verification");
    const auto scored = index.score(exact, [](const idx::object& x, const idx::object& y) {
        return double(x.support.source.identity.genome + y.support.source.identity.genome);
    }, 1);
    require(scored.score == 3 && scored.exact.distance == 1, "relation scoring distinct from verification");
    rejects([&] { index.score(index.verify({0, 3}, 1), [](const auto&, const auto&){ return 1.; }, 1); });
    rejects([&] { index.score(exact, [](const auto&, const auto&){
        return std::numeric_limits<double>::quiet_NaN(); }, 1); });
    rejects([&] { index.evaluate_recall(index.nominate(2, 0), 1); });
    rejects([&] { index.verify({1, 0}, 1); });
}
static void subscriptions_and_factors() {
    const Fixture input("ACNACAC");
    idx::sequence_index index({input.input(11, 2, idx::strand::reverse)}, 2);
    const auto range = index.equal_range("GT");
    require(range.second - range.first == 3, "reverse subscription exact truth");
    const auto source = index.posting_at(range.first).support.source;
    std::vector<idx::subscription> requests{{"GT", source, 41, 8, 100, 106},
        {"GT", source, 42, 9, 100, 106}};
    idx::subscription_answer answers[2]{};
    const auto count = index.subscribe(requests, answers, 2);
    require(count.required == 6 && count.stored == 2 && count.dropped == 4, "subscription fanout capacity");
    require(answers[0].destination == 41 && answers[0].question_role == 8
        && answers[0].support.source == source && answers[0].support.first > answers[0].support.last,
        "subscription source and destination role");
    require(index.subscribe(requests, nullptr, 0).required == 6, "zero capacity subscription");
    ++requests[0].expected_source.value_generation;
    rejects([&] { index.subscribe(requests, answers, 2); });
    std::array<Fixture, 3> roles{Fixture(std::string(19, 'A')), Fixture(std::string(21, 'C')),
        Fixture(std::string(23, 'G'))};
    idx::sequence_index factors({roles[0].input(1, 0), roles[1].input(2, 1), roles[2].input(3, 2)}, 1);
    std::array<std::vector<idx::factor_posting>, 3> postings;
    for (const auto& object : factors.objects()) {
        const auto role = object.support.source.role;
        postings[role].push_back({object.id, 7, double(role + 1)});
    }
    idx::factor output[3]{};
    const auto result = idx::join_factors(factors, postings, {1, 2, 3, 4}, output, 2);
    const auto expected = 19u * 21u * 23u;
    const double expected_score = 1 + 2 * 1 + 3 * 2 + 4 * 3;
    require(result.counts.required == expected && result.counts.stored == 2
        && result.counts.dropped == expected - 2, "factor cardinality/truncation");
    require(result.aggregates.size() == 1 && result.aggregates[0].count == expected
        && result.aggregates[0].score_sum == expected * expected_score, "CE factorized score oracle");
    for (std::size_t role = 0; role < 3; ++role)
        require(output[0].roles[role].source.role == role
            && output[0].roles[role].source.identity.genome == role + 1
            && output[0].score == expected_score, "ordered hyperedge role/provenance");
    require(idx::join_factors(factors, postings, {1, 2, 3, 4}, nullptr, 0).counts.required == expected,
        "zero capacity hyperedge");
    auto wrong = postings; std::swap(wrong[0], wrong[1]);
    rejects([&] { idx::join_factors(factors, wrong, {1, 2, 3, 4}, output, 2); });
    wrong = postings; wrong[0].push_back(wrong[0][0]);
    rejects([&] { idx::join_factors(factors, wrong, {1, 2, 3, 4}, output, 2); });
}
int main() {
    extraction_and_large_ranges(); recall_verification_scoring(); subscriptions_and_factors();
    rejects([] { idx::product(std::numeric_limits<idx::u64>::max(), 2); });
    std::cout << "global posting ranges, nomination/verification/score, subscriptions and CE factors passed\n";
}
