#pragma once

#include <Baseplane/seq/source_view.hh>
#include <Baseplane/seq/sequence_event.hh>
#include <cmath>
#include <variant>

namespace baseplane::query {

enum class support_domain : std::uint8_t { owned = 1, include_halos = 2 };

// Half-open support in the chosen strand's logical traversal, including invalid
// positions. An empty support at base_count is valid.
struct sequence_question {
    seq::source_stamp expected_source;
    std::uint32_t begin;
    std::uint32_t length;
    seq::source_strand strand;
    support_domain domain;
};
enum class question_status : std::uint8_t {
    ok, invalid_source, stale_source, invalid_support, invalid_output
};

inline question_status validate_question(const seq::source_view& source,
                                        const sequence_question& question) noexcept {
    if (!seq::valid_source(source)) return question_status::invalid_source;
    if (seq::stamp(source) != question.expected_source) return question_status::stale_source;
    if (!seq::valid_strand(question.strand)
        || (question.domain != support_domain::owned && question.domain != support_domain::include_halos)
        || !seq::dna2_window_fits_chunk(source.coordinates, question.begin, question.length))
        return question_status::invalid_support;
    return question_status::ok;
}

// Categories are separate types so learned confidence cannot silently satisfy
// an exact or certified answer. A certificate token identifies the provider's
// evidence; the structural validator does not prove its mathematical theorem.
struct exact_count { std::uint64_t value; };
struct certified_count_bound { double lower, upper; std::uint64_t certificate_id; };
struct learned_count { double prediction, confidence; std::uint64_t model_id, model_generation; };
struct detail_request { std::uint32_t begin, length; };
using answer_payload = std::variant<exact_count, certified_count_bound, learned_count, detail_request>;
struct sequence_answer { sequence_question question; answer_payload payload; };

inline bool same_question(const sequence_question& a, const sequence_question& b) noexcept {
    return a.expected_source == b.expected_source && a.begin == b.begin && a.length == b.length
        && a.strand == b.strand && a.domain == b.domain;
}
inline bool answer_matches(const seq::source_view& source, const sequence_question& question,
                           const sequence_answer& answer) noexcept {
    if (validate_question(source, question) != question_status::ok
        || !same_question(question, answer.question)) return false;
    if (const auto* value = std::get_if<exact_count>(&answer.payload))
        return value->value <= question.length;
    if (const auto* value = std::get_if<certified_count_bound>(&answer.payload))
        return value->certificate_id != 0 && std::isfinite(value->lower) && std::isfinite(value->upper)
            && value->lower >= 0 && value->lower <= value->upper && value->upper <= question.length;
    if (const auto* value = std::get_if<learned_count>(&answer.payload))
        return value->model_id != 0 && std::isfinite(value->prediction)
            && value->prediction >= 0 && value->prediction <= question.length
            && std::isfinite(value->confidence) && value->confidence >= 0 && value->confidence <= 1;
    const auto* request = std::get_if<detail_request>(&answer.payload);
    return request && request->begin >= question.begin
        && request->begin <= question.begin + question.length
        && request->length <= question.begin + question.length - request->begin;
}

struct detail_record {
    std::uint64_t global_position;
    std::uint32_t logical_position, physical_position;
    std::uint8_t code;
    seq::source_strand strand;
    bool owned;
};
struct detail_result {
    question_status status;
    seq::source_stamp source;
    seq::sequence_emit_counts counts;
    std::uint64_t examined, invalid_positions, excluded_halos;
};

// Caller-owned host output, ordered by logical position (reverse strand has
// decreasing global coordinates). Invalid bases and unowned halo anchors never
// become valid owned detail. Null output is permitted only for capacity zero.
inline detail_result emit_exact_detail(const seq::source_view& source,
                                       const sequence_question& question,
                                       detail_record* output, std::uint64_t capacity) noexcept {
    detail_result result{validate_question(source, question), seq::stamp(source), {}, 0, 0, 0};
    if (result.status != question_status::ok) return result;
    if (capacity && !output) { result.status = question_status::invalid_output; return result; }
    for (std::uint32_t offset = 0; offset < question.length; ++offset) {
        const auto logical = question.begin + offset;
        seq::source_base base{};
        seq::read_source_base(source, question.strand, logical, &base);
        ++result.examined;
        if (!base.valid) { ++result.invalid_positions; continue; }
        if (!base.owned && question.domain == support_domain::owned) {
            ++result.excluded_halos; continue;
        }
        ++result.counts.total_matches;
        if (result.counts.stored_records < capacity) {
            output[result.counts.stored_records++] = {base.global_position, logical,
                base.physical_position, base.code, question.strand, base.owned};
        }
    }
    result.counts.required_capacity = result.counts.total_matches;
    result.counts.dropped_records = result.counts.total_matches - result.counts.stored_records;
    return result;
}

// Exact scalar count over the same support/domain without allocating detail.
inline bool count_valid_source(const seq::source_view& source, const sequence_question& question,
                               sequence_answer* answer) noexcept {
    if (!answer) return false;
    const auto result = emit_exact_detail(source, question, nullptr, 0);
    if (result.status != question_status::ok) return false;
    *answer = {question, exact_count{result.counts.total_matches}};
    return true;
}

} // namespace baseplane::query
