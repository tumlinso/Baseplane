#pragma once

#include <Baseplane/seq/dna2_motif.hh>

#include <cstddef>
#include <cstdint>
#include <type_traits>

#if defined(__CUDACC__)
#define BASEPLANE_SEQUENCE_EVENT_HD __host__ __device__
#else
#define BASEPLANE_SEQUENCE_EVENT_HD
#endif

namespace baseplane::seq {

// Compact Baseplane-local record. Biological axes, chunk origin, ownership,
// halos, residency, scores, and segment offsets are descriptor/sidecar data.
struct sequence_event {
    std::uint32_t local_position;
    std::uint16_t predicate_id;
    std::uint8_t payload;
    std::uint8_t flags;
};

inline constexpr std::uint8_t sequence_event_forward_strand = 0x1u;
inline constexpr std::uint8_t sequence_event_reverse_strand = 0x2u;

enum class sequence_output_mode : std::uint8_t {
    count = 1u,
    mask = 2u,
    stable_emit = 3u,
    unordered_emit = 4u,
};

enum class sequence_event_ordering : std::uint8_t {
    not_applicable = 0u,
    coordinate_stable = 1u,
    unordered = 2u,
};

// This is execution metadata, not biological identity. A Cellerator adapter
// maps it to the authoritative cellerator-biological-abi-v1 location record.
enum class sequence_buffer_residency : std::uint8_t {
    host = 1u,
    device = 2u,
    managed = 3u,
    peer_device = 4u,
};

struct sequence_emit_counts {
    std::uint64_t total_matches;
    std::uint64_t stored_records;
    std::uint64_t dropped_records;
    std::uint64_t required_capacity;
};

// All pointers refer to one declared residency. For count mode no data buffer
// is present. Mask mode uses mask_words. Emit modes use records. Counters are
// required for every mode and remain caller-owned.
struct sequence_output_descriptor {
    sequence_event* records;
    std::uint32_t* mask_words;
    sequence_emit_counts* counts;
    std::uint64_t capacity;
    std::uint64_t mask_word_count;
    std::int32_t device_ordinal;
    sequence_output_mode mode;
    sequence_event_ordering ordering;
    sequence_buffer_residency residency;
    std::uint8_t reserved;
};

BASEPLANE_SEQUENCE_EVENT_HD constexpr sequence_event sequence_event_from_motif_hit(
    motif_hit hit) noexcept {
    return sequence_event{hit.position, hit.motif_id, hit.mismatches, hit.strand};
}

BASEPLANE_SEQUENCE_EVENT_HD constexpr motif_hit motif_hit_from_sequence_event(
    sequence_event event) noexcept {
    return motif_hit{
        event.local_position, event.predicate_id, event.payload, event.flags};
}

BASEPLANE_SEQUENCE_EVENT_HD constexpr bool sequence_buffer_location_valid(
    sequence_buffer_residency residency,
    std::int32_t device_ordinal) noexcept {
    if (residency == sequence_buffer_residency::host)
        return device_ordinal == -1;
    return (residency == sequence_buffer_residency::device
            || residency == sequence_buffer_residency::managed
            || residency == sequence_buffer_residency::peer_device)
        && device_ordinal >= 0;
}

BASEPLANE_SEQUENCE_EVENT_HD constexpr bool sequence_emit_counts_valid(
    sequence_output_mode mode,
    std::uint64_t capacity,
    const sequence_emit_counts& counts) noexcept {
    if (counts.required_capacity != counts.total_matches)
        return false;
    if (mode == sequence_output_mode::count || mode == sequence_output_mode::mask)
        return counts.stored_records == 0u && counts.dropped_records == 0u;
    if (mode != sequence_output_mode::stable_emit
        && mode != sequence_output_mode::unordered_emit)
        return false;
    if (counts.stored_records > capacity
        || counts.stored_records > counts.total_matches)
        return false;
    return counts.dropped_records
        == counts.total_matches - counts.stored_records;
}

BASEPLANE_SEQUENCE_EVENT_HD constexpr bool sequence_output_descriptor_valid(
    const sequence_output_descriptor& output) noexcept {
    if (output.counts == nullptr
        || !sequence_buffer_location_valid(output.residency, output.device_ordinal))
        return false;
    if (output.mode == sequence_output_mode::count)
        return output.records == nullptr && output.mask_words == nullptr
            && output.capacity == 0u && output.mask_word_count == 0u
            && output.ordering == sequence_event_ordering::not_applicable;
    if (output.mode == sequence_output_mode::mask)
        return output.records == nullptr && output.capacity == 0u
            && (output.mask_word_count == 0u || output.mask_words != nullptr)
            && output.ordering == sequence_event_ordering::not_applicable;
    if (output.mode == sequence_output_mode::stable_emit)
        return output.mask_words == nullptr && output.mask_word_count == 0u
            && (output.capacity == 0u || output.records != nullptr)
            && output.ordering == sequence_event_ordering::coordinate_stable;
    if (output.mode == sequence_output_mode::unordered_emit)
        return output.mask_words == nullptr && output.mask_word_count == 0u
            && (output.capacity == 0u || output.records != nullptr)
            && output.ordering == sequence_event_ordering::unordered;
    return false;
}

static_assert(sizeof(sequence_event) == 8u,
    "sequence event v1 core must remain eight bytes");
static_assert(sizeof(sequence_event) == sizeof(motif_hit),
    "sequence event and exact motif hit must remain layout-compatible in v1");
static_assert(alignof(sequence_event) == alignof(motif_hit),
    "sequence event and exact motif hit alignment diverged");
static_assert(offsetof(sequence_event, local_position) == offsetof(motif_hit, position));
static_assert(offsetof(sequence_event, predicate_id) == offsetof(motif_hit, motif_id));
static_assert(offsetof(sequence_event, payload) == offsetof(motif_hit, mismatches));
static_assert(offsetof(sequence_event, flags) == offsetof(motif_hit, strand));
static_assert(std::is_trivially_copyable_v<sequence_event>);
static_assert(std::is_standard_layout_v<sequence_event>);
static_assert(std::is_trivially_copyable_v<sequence_emit_counts>);
static_assert(std::is_trivially_copyable_v<sequence_output_descriptor>);

} // namespace baseplane::seq

#undef BASEPLANE_SEQUENCE_EVENT_HD
