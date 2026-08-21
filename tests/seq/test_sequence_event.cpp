#include <Baseplane/seq/sequence_event.hh>

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace seq = ::baseplane::seq;

namespace {

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

static_assert(sizeof(seq::sequence_event) == 8u);
static_assert(alignof(seq::sequence_event) == alignof(seq::motif_hit));
static_assert(std::is_standard_layout_v<seq::sequence_event>);
static_assert(std::is_trivially_copyable_v<seq::sequence_event>);
static_assert(std::is_trivially_copyable_v<seq::sequence_output_descriptor>);
static_assert(offsetof(seq::sequence_event, local_position) == 0u);
static_assert(offsetof(seq::sequence_event, predicate_id) == 4u);
static_assert(offsetof(seq::sequence_event, payload) == 6u);
static_assert(offsetof(seq::sequence_event, flags) == 7u);

void test_motif_hit_compatibility() {
    constexpr seq::motif_hit hit{0xffffffffu, 0xffffu, 0x7fu, 0x3u};
    constexpr seq::sequence_event event = seq::sequence_event_from_motif_hit(hit);
    static_assert(event.local_position == hit.position);
    static_assert(event.predicate_id == hit.motif_id);
    static_assert(event.payload == hit.mismatches);
    static_assert(event.flags == hit.strand);
    constexpr seq::motif_hit restored = seq::motif_hit_from_sequence_event(event);
    static_assert(restored.position == hit.position);
    static_assert(restored.motif_id == hit.motif_id);
    static_assert(restored.mismatches == hit.mismatches);
    static_assert(restored.strand == hit.strand);
}

void test_locations_and_descriptors() {
    seq::sequence_emit_counts counts{};
    seq::sequence_event records[2]{};
    std::uint32_t masks[2]{};

    require(seq::sequence_output_descriptor_valid(seq::sequence_output_descriptor{
                nullptr, nullptr, &counts, 0u, 0u, -1,
                seq::sequence_output_mode::count,
                seq::sequence_event_ordering::not_applicable,
                seq::sequence_buffer_residency::host, 0u}),
            "valid host count descriptor rejected");
    require(seq::sequence_output_descriptor_valid(seq::sequence_output_descriptor{
                nullptr, masks, &counts, 0u, 2u, 0,
                seq::sequence_output_mode::mask,
                seq::sequence_event_ordering::not_applicable,
                seq::sequence_buffer_residency::device, 0u}),
            "valid device mask descriptor rejected");
    require(seq::sequence_output_descriptor_valid(seq::sequence_output_descriptor{
                records, nullptr, &counts, 2u, 0u, -1,
                seq::sequence_output_mode::stable_emit,
                seq::sequence_event_ordering::coordinate_stable,
                seq::sequence_buffer_residency::host, 0u}),
            "valid stable emit descriptor rejected");
    require(seq::sequence_output_descriptor_valid(seq::sequence_output_descriptor{
                records, nullptr, &counts, 2u, 0u, 3,
                seq::sequence_output_mode::unordered_emit,
                seq::sequence_event_ordering::unordered,
                seq::sequence_buffer_residency::peer_device, 0u}),
            "valid unordered peer descriptor rejected");
    require(seq::sequence_output_descriptor_valid(seq::sequence_output_descriptor{
                nullptr, nullptr, &counts, 0u, 0u, 0,
                seq::sequence_output_mode::unordered_emit,
                seq::sequence_event_ordering::unordered,
                seq::sequence_buffer_residency::device, 0u}),
            "zero-capacity emit descriptor rejected");

    require(!seq::sequence_output_descriptor_valid(seq::sequence_output_descriptor{
                nullptr, nullptr, nullptr, 0u, 0u, -1,
                seq::sequence_output_mode::count,
                seq::sequence_event_ordering::not_applicable,
                seq::sequence_buffer_residency::host, 0u}),
            "null counters accepted");
    require(!seq::sequence_output_descriptor_valid(seq::sequence_output_descriptor{
                records, nullptr, &counts, 2u, 0u, -1,
                seq::sequence_output_mode::stable_emit,
                seq::sequence_event_ordering::unordered,
                seq::sequence_buffer_residency::host, 0u}),
            "stable emit with unordered promise accepted");
    require(!seq::sequence_output_descriptor_valid(seq::sequence_output_descriptor{
                nullptr, masks, &counts, 0u, 2u, 0,
                seq::sequence_output_mode::mask,
                seq::sequence_event_ordering::not_applicable,
                seq::sequence_buffer_residency::host, 0u}),
            "host residency with device ordinal accepted");
}

void test_count_accounting() {
    require(seq::sequence_emit_counts_valid(
                seq::sequence_output_mode::count, 0u,
                seq::sequence_emit_counts{7u, 0u, 0u, 7u}),
            "count-only accounting rejected");
    require(seq::sequence_emit_counts_valid(
                seq::sequence_output_mode::mask, 0u,
                seq::sequence_emit_counts{9u, 0u, 0u, 9u}),
            "mask accounting rejected");
    require(seq::sequence_emit_counts_valid(
                seq::sequence_output_mode::stable_emit, 3u,
                seq::sequence_emit_counts{5u, 3u, 2u, 5u}),
            "truncated stable accounting rejected");
    require(seq::sequence_emit_counts_valid(
                seq::sequence_output_mode::unordered_emit, 8u,
                seq::sequence_emit_counts{5u, 5u, 0u, 5u}),
            "complete unordered accounting rejected");
    require(seq::sequence_emit_counts_valid(
                seq::sequence_output_mode::stable_emit, 0u,
                seq::sequence_emit_counts{5u, 0u, 5u, 5u}),
            "zero-capacity accounting rejected");

    require(!seq::sequence_emit_counts_valid(
                seq::sequence_output_mode::stable_emit, 3u,
                seq::sequence_emit_counts{5u, 4u, 1u, 5u}),
            "stored records beyond capacity accepted");
    require(!seq::sequence_emit_counts_valid(
                seq::sequence_output_mode::unordered_emit, 3u,
                seq::sequence_emit_counts{5u, 3u, 1u, 5u}),
            "silent truncation accounting accepted");
    require(!seq::sequence_emit_counts_valid(
                seq::sequence_output_mode::count, 0u,
                seq::sequence_emit_counts{5u, 0u, 0u, 4u}),
            "incorrect required capacity accepted");
}

} // namespace

int main() {
    test_motif_hit_compatibility();
    test_locations_and_descriptors();
    test_count_accounting();
    return 0;
}
