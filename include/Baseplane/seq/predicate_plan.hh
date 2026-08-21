#pragma once

#include <Baseplane/seq/dna2_allowed.hh>
#include <Baseplane/seq/dna2_validity.hh>
#include <Baseplane/seq/sequence_event.hh>

#include <cstdint>
#include <type_traits>

namespace baseplane::seq {

// Exact public ABI consumed by Cellerator's sequence integration. This is a
// narrow sequence/predicate contract, not a package-wide ABI registry.
inline constexpr std::uint32_t sequence_predicate_abi_version = 1u;
inline constexpr std::uint16_t sequence_program_version = 1u;
inline constexpr std::uint16_t sequence_program_no_input = 0xffffu;
inline constexpr std::uint16_t sequence_program_max_nodes = 64u;
inline constexpr std::uint16_t sequence_program_max_outputs = 16u;
inline constexpr std::uint8_t sequence_program_max_motifs = 16u;
inline constexpr std::uint32_t sequence_program_max_span = 32u;

enum class predicate_opcode : std::uint8_t {
    base_equal = 1u,
    allowed_motif = 2u,
    exact_motif = 3u,
    gc_mask = 4u,
    base_class_mask = 5u,
    cpg_start_mask = 6u,
    shift = 7u,
    bit_and = 8u,
    bit_or = 9u,
    bit_xor = 10u,
    bit_and_not = 11u,
    bit_not = 12u,
    any = 13u,
    all = 14u,
    popcount_threshold = 15u,
    spacing_exact = 16u,
    spacing_within = 17u,
    run = 18u,
    boundary = 19u,
    emit_event = 20u,
    segment_reduce_request = 21u,
};

enum class predicate_value_kind : std::uint8_t {
    mask = 1u,
    scalar_count = 2u,
    event = 3u,
    segment_request = 4u,
};

enum class predicate_plan_status : std::uint8_t {
    ok = 0u,
    null_output = 1u,
    unsupported_version = 2u,
    invalid_count = 3u,
    invalid_opcode = 4u,
    invalid_input = 5u,
    invalid_immediate = 6u,
    invalid_motif = 7u,
    invalid_output = 8u,
};

enum class predicate_lowering_family : std::uint8_t {
    exact_scan = 1u,
    fused_plane_mask = 2u,
    segment = 3u,
    scalar_reference = 4u,
};

inline constexpr std::uint8_t predicate_shift_right = 0x1u;

struct predicate_instruction {
    predicate_opcode opcode;
    predicate_value_kind result_kind;
    std::uint8_t flags;
    std::uint8_t reserved;
    std::uint16_t input_a;
    std::uint16_t input_b;
    std::uint32_t immediate;
    std::uint32_t auxiliary;
};

struct predicate_output {
    std::uint16_t node;
    std::uint16_t predicate_id;
    sequence_output_mode mode;
    std::uint8_t payload;
    std::uint8_t flags;
    std::uint8_t reserved;
};

struct sequence_predicate_program {
    std::uint16_t version;
    std::uint16_t node_count;
    std::uint16_t output_count;
    std::uint8_t exact_motif_count;
    std::uint8_t allowed_motif_count;
    std::uint32_t semantic_flags;
    predicate_instruction nodes[sequence_program_max_nodes];
    predicate_output outputs[sequence_program_max_outputs];
    motif32_exact exact_motifs[sequence_program_max_motifs];
    motif32_allowed allowed_motifs[sequence_program_max_motifs];
};

struct prepared_predicate_plan {
    std::uint16_t version;
    std::uint16_t node_count;
    std::uint16_t live_node_count;
    std::uint16_t output_count;
    std::uint32_t lookbehind;
    std::uint32_t lookahead;
    std::uint32_t scratch_mask_words_per_window;
    predicate_lowering_family family;
    std::uint8_t reserved[3];
    std::uint64_t semantic_hash;
    std::uint16_t original_to_prepared[sequence_program_max_nodes];
};

predicate_plan_status verify_sequence_predicate_program(
    const sequence_predicate_program& program) noexcept;
std::uint64_t sequence_predicate_program_hash(
    const sequence_predicate_program& program) noexcept;
predicate_plan_status prepare_sequence_predicate_program(
    const sequence_predicate_program& program,
    prepared_predicate_plan* prepared) noexcept;

static_assert(std::is_trivially_copyable_v<predicate_instruction>);
static_assert(std::is_trivially_copyable_v<sequence_predicate_program>);
static_assert(std::is_trivially_copyable_v<prepared_predicate_plan>);
static_assert(std::is_standard_layout_v<sequence_predicate_program>);
static_assert(std::is_standard_layout_v<prepared_predicate_plan>);

} // namespace baseplane::seq
