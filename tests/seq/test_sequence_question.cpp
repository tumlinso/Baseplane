#include <Baseplane/query/contracts/sequence_question.hh>
#include <array>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace seq = baseplane::seq;
namespace query = baseplane::query;
static void require(bool value, const char* message) {
    if (!value) throw std::runtime_error(message);
}

struct Fixture {
    std::string bases;
    std::vector<std::uint64_t> packed, valid;
    explicit Fixture(std::size_t length) : bases(length, 'A'),
        packed(seq::dna2_packed64_word_count(length), 0),
        valid(seq::dna2_validity64_word_count(length), 0) {
        const char alphabet[] = "ACGTNacgt-";
        for (std::size_t i = 0; i < length; ++i) {
            bases[i] = alphabet[(i * 7 + length) % 10];
            const auto base = seq::dna2_encode_base_with_validity(bases[i]);
            // Deliberately put arbitrary nonzero payload under invalid sidecars.
            const auto code = base.valid ? base.code : 3u;
            packed[i / 32] |= std::uint64_t{code} << (2 * (i % 32));
            if (base.valid) valid[i / 64] |= std::uint64_t{1} << (i % 64);
        }
        // Pollute tail payload/sidecar to ensure bounds remain authoritative.
        if (length % 32 && !packed.empty()) packed.back() |= ~std::uint64_t{0} << (2 * (length % 32));
        if (length % 64 && !valid.empty()) valid.back() |= ~std::uint64_t{0} << (length % 64);
    }
    seq::source_view view() const {
        const auto size = static_cast<std::uint32_t>(bases.size());
        const auto left = size ? 1u : 0u;
        const auto end = size > 1 ? size - 1 : left;
        return {{{packed.data(), size, packed.size()}, valid.data(), valid.size()},
            {{19, 4, 8}, 1000, size, left, end, left, size - end}, 3, 11};
    }
};

// Independent text oracle: does not use packed decode, source mapping or validity helpers.
static int text_code(char base) {
    switch (base) {
        case 'A': case 'a': return 0;
        case 'C': case 'c': return 1;
        case 'G': case 'g': return 2;
        case 'T': case 't': return 3;
        default: return -1;
    }
}

static void compare_scalar(const Fixture& fixture, seq::source_strand strand,
                           query::support_domain domain, std::uint32_t begin,
                           std::uint32_t length, std::uint64_t capacity) {
    const auto source = fixture.view();
    const query::sequence_question question{seq::stamp(source), begin, length, strand, domain};
    std::vector<query::detail_record> expected;
    std::uint64_t invalid = 0, excluded = 0;
    for (std::uint32_t offset = 0; offset < length; ++offset) {
        const auto logical = begin + offset;
        const auto physical = strand == seq::source_strand::forward ? logical
            : static_cast<std::uint32_t>(fixture.bases.size()) - 1 - logical;
        const auto code = text_code(fixture.bases[physical]);
        const bool owned = physical >= source.coordinates.owned_begin
            && physical < source.coordinates.owned_end;
        if (code < 0) { ++invalid; continue; }
        if (!owned && domain == query::support_domain::owned) { ++excluded; continue; }
        expected.push_back({1000u + physical, logical, physical,
            static_cast<std::uint8_t>(strand == seq::source_strand::forward ? code : code ^ 3),
            strand, owned});
    }
    // A sentinel just beyond capacity must survive truncation.
    std::vector<query::detail_record> output(capacity + 1);
    output.back().global_position = 999999;
    const auto result = query::emit_exact_detail(source, question, capacity ? output.data() : nullptr, capacity);
    require(result.status == query::question_status::ok, "valid question rejected");
    require(result.source == seq::stamp(source), "provenance lost");
    require(result.examined == length && result.invalid_positions == invalid
        && result.excluded_halos == excluded, "support accounting");
    const auto stored = std::min<std::uint64_t>(capacity, expected.size());
    require(result.counts.total_matches == expected.size() && result.counts.required_capacity == expected.size()
        && result.counts.stored_records == stored && result.counts.dropped_records == expected.size() - stored,
        "capacity accounting");
    require(seq::sequence_emit_counts_valid(seq::sequence_output_mode::stable_emit, capacity, result.counts),
        "existing capacity contract");
    require(output.back().global_position == 999999, "capacity overwrite");
    for (std::size_t i = 0; i < stored; ++i) {
        require(output[i].global_position == expected[i].global_position
            && output[i].logical_position == expected[i].logical_position
            && output[i].physical_position == expected[i].physical_position
            && output[i].code == expected[i].code && output[i].strand == expected[i].strand
            && output[i].owned == expected[i].owned, "independent scalar detail comparison");
    }
    query::sequence_answer answer{};
    require(query::count_valid_source(source, question, &answer), "count failed");
    require(std::get<query::exact_count>(answer.payload).value == expected.size(), "scalar exact count");
    require(query::answer_matches(source, question, answer), "count context mismatch");
}

static void rejection_and_answers() {
    const Fixture fixture(65);
    auto source = fixture.view();
    query::sequence_question question{seq::stamp(source), 0, 65, seq::source_strand::forward,
        query::support_domain::owned};
    seq::source_base base{777, 777, 7, true, true};
    require(!seq::read_source_base(source, question.strand, 65, &base) && base.global_position == 777,
        "tail read changed output");
    require(!seq::read_source_base(source, static_cast<seq::source_strand>(3), 0, &base), "unknown strand");
    require(!seq::read_source_base(source, question.strand, 0, nullptr), "null base output");
    // Find a genuinely invalid source position; its nonzero packed payload is hidden.
    for (std::uint32_t i = 0; i < fixture.bases.size(); ++i) {
        if (text_code(fixture.bases[i]) >= 0) continue;
        require(seq::read_source_base(source, question.strand, i, &base) && !base.valid && base.code == 0,
            "invalid payload decoded");
    }
    require(query::emit_exact_detail(source, question, nullptr, 1).status
        == query::question_status::invalid_output, "null capacity output");
    auto stale = question;
    ++stale.expected_source.value_generation;
    require(query::validate_question(source, stale) == query::question_status::stale_source, "stale value accepted");
    stale = question; ++stale.expected_source.structure_epoch;
    require(query::validate_question(source, stale) == query::question_status::stale_source, "stale structure accepted");
    stale = question; ++stale.expected_source.identity.contig;
    require(query::validate_question(source, stale) == query::question_status::stale_source, "wrong contig");
    auto unsupported = question; unsupported.begin = 65; unsupported.length = 1;
    require(query::validate_question(source, unsupported) == query::question_status::invalid_support, "out of range");
    unsupported.begin = std::numeric_limits<std::uint32_t>::max();
    require(query::validate_question(source, unsupported) == query::question_status::invalid_support, "overflow support");
    unsupported = question; unsupported.domain = static_cast<query::support_domain>(0);
    require(query::validate_question(source, unsupported) == query::question_status::invalid_support, "unknown domain");
    auto malformed = source; malformed.coordinates.halo_right = 3;
    require(query::validate_question(malformed, question) == query::question_status::invalid_source, "bad halos");
    malformed = source; malformed.coordinates.global_base_begin = std::numeric_limits<std::uint64_t>::max();
    require(!seq::valid_source(malformed), "coordinate overflow");
    malformed = source; --malformed.sequence.packed.n_words;
    require(!seq::valid_source(malformed), "short packed allocation");
    malformed = source; --malformed.sequence.n_validity_words;
    require(!seq::valid_source(malformed), "short validity allocation");
    malformed = source; --malformed.sequence.packed.n_bases;
    require(!seq::valid_source(malformed), "source length mismatch");

    query::sequence_answer answer{question, query::certified_count_bound{0, 65, 7}};
    require(query::answer_matches(source, question, answer), "certified category rejected");
    require(!std::holds_alternative<query::exact_count>(answer.payload), "bound conflated with exact");
    answer.payload = query::certified_count_bound{0, 65, 0};
    require(!query::answer_matches(source, question, answer), "missing certificate");
    answer.payload = query::certified_count_bound{4, 3, 1};
    require(!query::answer_matches(source, question, answer), "inverted bound");
    answer.payload = query::learned_count{3.5, .8, 6, 9};
    require(query::answer_matches(source, question, answer)
        && !std::holds_alternative<query::certified_count_bound>(answer.payload), "confidence category");
    answer.payload = query::learned_count{3, 1.1, 6, 9};
    require(!query::answer_matches(source, question, answer), "out of range confidence");
    answer.payload = query::learned_count{std::numeric_limits<double>::quiet_NaN(), .5, 6, 9};
    require(!query::answer_matches(source, question, answer), "NaN prediction");
    answer.payload = query::detail_request{31, 3};
    require(query::answer_matches(source, question, answer), "detail support rejected");
    answer.payload = query::detail_request{64, 2};
    require(!query::answer_matches(source, question, answer), "detail outside support");
    answer.payload = query::exact_count{66};
    require(!query::answer_matches(source, question, answer), "impossible count");
    answer.payload = query::exact_count{1};
    auto reverse = question; reverse.strand = seq::source_strand::reverse;
    require(!query::answer_matches(source, reverse, answer), "wrong question strand");
    ++source.value_generation;
    require(!query::answer_matches(source, question, answer), "stale answer");
}

int main() {
    for (const auto length : {0u, 1u, 2u, 31u, 32u, 33u, 63u, 64u, 65u, 127u, 129u}) {
        const Fixture fixture(length);
        require(seq::valid_source(fixture.view()), "fixture invalid");
        for (const auto strand : {seq::source_strand::forward, seq::source_strand::reverse})
            for (const auto domain : {query::support_domain::owned, query::support_domain::include_halos})
                for (const auto capacity : {0u, 1u, 3u, 200u}) {
                    compare_scalar(fixture, strand, domain, 0, length, capacity);
                    compare_scalar(fixture, strand, domain, length, 0, capacity);
                    if (length > 2) compare_scalar(fixture, strand, domain, 1, length - 2, capacity);
                }
    }
    rejection_and_answers();
    std::cout << "sequence source and question contracts: passed\n";
}
