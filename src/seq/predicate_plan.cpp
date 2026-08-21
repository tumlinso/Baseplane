#include <Baseplane/seq/predicate_plan.hh>

#include <cstdint>

namespace baseplane::seq {
namespace {

bool source_opcode(predicate_opcode opcode) noexcept {
    return opcode == predicate_opcode::base_equal
        || opcode == predicate_opcode::allowed_motif
        || opcode == predicate_opcode::exact_motif
        || opcode == predicate_opcode::gc_mask
        || opcode == predicate_opcode::base_class_mask
        || opcode == predicate_opcode::cpg_start_mask;
}

bool unary_opcode(predicate_opcode opcode) noexcept {
    return opcode == predicate_opcode::shift || opcode == predicate_opcode::bit_not
        || opcode == predicate_opcode::any || opcode == predicate_opcode::all
        || opcode == predicate_opcode::popcount_threshold
        || opcode == predicate_opcode::run || opcode == predicate_opcode::boundary
        || opcode == predicate_opcode::emit_event
        || opcode == predicate_opcode::segment_reduce_request;
}

bool binary_opcode(predicate_opcode opcode) noexcept {
    return opcode == predicate_opcode::bit_and || opcode == predicate_opcode::bit_or
        || opcode == predicate_opcode::bit_xor || opcode == predicate_opcode::bit_and_not
        || opcode == predicate_opcode::spacing_exact
        || opcode == predicate_opcode::spacing_within;
}

bool valid_opcode(predicate_opcode opcode) noexcept {
    return source_opcode(opcode) || unary_opcode(opcode) || binary_opcode(opcode);
}

bool valid_value_kind(predicate_value_kind kind) noexcept {
    return kind == predicate_value_kind::mask
        || kind == predicate_value_kind::scalar_count
        || kind == predicate_value_kind::event
        || kind == predicate_value_kind::segment_request;
}

void hash_byte(std::uint64_t& hash, std::uint8_t value) noexcept {
    hash ^= value;
    hash *= 1099511628211ULL;
}

template<typename T>
void hash_unsigned(std::uint64_t& hash, T value) noexcept {
    for (unsigned shift = 0u; shift < sizeof(T) * 8u; shift += 8u)
        hash_byte(hash, static_cast<std::uint8_t>(value >> shift));
}

void mark_live(
    const sequence_predicate_program& program,
    std::uint16_t node,
    bool* live) noexcept {
    if (node >= program.node_count || live[node]) return;
    live[node] = true;
    const predicate_instruction& instruction = program.nodes[node];
    if (instruction.input_a != sequence_program_no_input)
        mark_live(program, instruction.input_a, live);
    if (instruction.input_b != sequence_program_no_input)
        mark_live(program, instruction.input_b, live);
}

} // namespace

predicate_plan_status verify_sequence_predicate_program(
    const sequence_predicate_program& program) noexcept {
    if (program.version != sequence_program_version)
        return predicate_plan_status::unsupported_version;
    if (program.node_count == 0u || program.node_count > sequence_program_max_nodes
        || program.output_count == 0u || program.output_count > sequence_program_max_outputs
        || program.exact_motif_count > sequence_program_max_motifs
        || program.allowed_motif_count > sequence_program_max_motifs)
        return predicate_plan_status::invalid_count;
    for (std::uint8_t index = 0u; index < program.exact_motif_count; ++index)
        if (!dna2_valid_motif32_exact(program.exact_motifs[index]))
            return predicate_plan_status::invalid_motif;
    for (std::uint8_t index = 0u; index < program.allowed_motif_count; ++index)
        if (!dna2_valid_motif32_allowed(program.allowed_motifs[index]))
            return predicate_plan_status::invalid_motif;

    for (std::uint16_t index = 0u; index < program.node_count; ++index) {
        const predicate_instruction& node = program.nodes[index];
        if (!valid_opcode(node.opcode) || !valid_value_kind(node.result_kind))
            return predicate_plan_status::invalid_opcode;
        const bool no_a = node.input_a == sequence_program_no_input;
        const bool no_b = node.input_b == sequence_program_no_input;
        if ((source_opcode(node.opcode) && (!no_a || !no_b))
            || (unary_opcode(node.opcode) && (no_a || !no_b || node.input_a >= index))
            || (binary_opcode(node.opcode)
                && (no_a || no_b || node.input_a >= index || node.input_b >= index)))
            return predicate_plan_status::invalid_input;
        if (node.opcode == predicate_opcode::base_equal && node.immediate > 3u)
            return predicate_plan_status::invalid_immediate;
        if (node.opcode == predicate_opcode::exact_motif
            && node.immediate >= program.exact_motif_count)
            return predicate_plan_status::invalid_motif;
        if (node.opcode == predicate_opcode::allowed_motif
            && node.immediate >= program.allowed_motif_count)
            return predicate_plan_status::invalid_motif;
        if ((node.opcode == predicate_opcode::shift
                || node.opcode == predicate_opcode::any
                || node.opcode == predicate_opcode::all
                || node.opcode == predicate_opcode::spacing_exact
                || node.opcode == predicate_opcode::spacing_within
                || node.opcode == predicate_opcode::run)
            && (node.immediate == 0u || node.immediate > sequence_program_max_span))
            return predicate_plan_status::invalid_immediate;
        if (node.opcode == predicate_opcode::popcount_threshold
            && node.immediate > sequence_program_max_span)
            return predicate_plan_status::invalid_immediate;
    }
    for (std::uint16_t index = 0u; index < program.output_count; ++index) {
        const predicate_output& output = program.outputs[index];
        if (output.node >= program.node_count
            || (output.mode != sequence_output_mode::count
                && output.mode != sequence_output_mode::mask
                && output.mode != sequence_output_mode::stable_emit
                && output.mode != sequence_output_mode::unordered_emit))
            return predicate_plan_status::invalid_output;
    }
    return predicate_plan_status::ok;
}

std::uint64_t sequence_predicate_program_hash(
    const sequence_predicate_program& program) noexcept {
    std::uint64_t hash = 1469598103934665603ULL;
    hash_unsigned(hash, program.version);
    hash_unsigned(hash, program.node_count);
    hash_unsigned(hash, program.output_count);
    hash_unsigned(hash, program.exact_motif_count);
    hash_unsigned(hash, program.allowed_motif_count);
    hash_unsigned(hash, program.semantic_flags);
    for (std::uint16_t index = 0u; index < program.node_count; ++index) {
        const predicate_instruction& node = program.nodes[index];
        hash_unsigned(hash, static_cast<std::uint8_t>(node.opcode));
        hash_unsigned(hash, static_cast<std::uint8_t>(node.result_kind));
        hash_unsigned(hash, node.flags);
        hash_unsigned(hash, node.input_a);
        hash_unsigned(hash, node.input_b);
        hash_unsigned(hash, node.immediate);
        hash_unsigned(hash, node.auxiliary);
    }
    for (std::uint16_t index = 0u; index < program.output_count; ++index) {
        const predicate_output& output = program.outputs[index];
        hash_unsigned(hash, output.node);
        hash_unsigned(hash, output.predicate_id);
        hash_unsigned(hash, static_cast<std::uint8_t>(output.mode));
        hash_unsigned(hash, output.payload);
        hash_unsigned(hash, output.flags);
    }
    for (std::uint8_t index = 0u; index < program.exact_motif_count; ++index) {
        const motif32_exact& motif = program.exact_motifs[index];
        hash_unsigned(hash, motif.packed);
        hash_unsigned(hash, motif.active_fields);
        hash_unsigned(hash, motif.length);
        hash_unsigned(hash, motif.max_mismatches);
        hash_unsigned(hash, motif.motif_id);
    }
    for (std::uint8_t index = 0u; index < program.allowed_motif_count; ++index) {
        const motif32_allowed& motif = program.allowed_motifs[index];
        hash_unsigned(hash, motif.allow_A); hash_unsigned(hash, motif.allow_C);
        hash_unsigned(hash, motif.allow_G); hash_unsigned(hash, motif.allow_T);
        hash_unsigned(hash, motif.active_mask); hash_unsigned(hash, motif.length);
        hash_unsigned(hash, motif.max_mismatches); hash_unsigned(hash, motif.motif_id);
        hash_unsigned(hash, static_cast<std::uint8_t>(motif.strand_mode));
        hash_unsigned(hash, static_cast<std::uint8_t>(motif.palindrome_policy));
    }
    return hash;
}

predicate_plan_status prepare_sequence_predicate_program(
    const sequence_predicate_program& program,
    prepared_predicate_plan* prepared) noexcept {
    if (prepared == nullptr) return predicate_plan_status::null_output;
    const predicate_plan_status status = verify_sequence_predicate_program(program);
    if (status != predicate_plan_status::ok) return status;

    bool live[sequence_program_max_nodes]{};
    for (std::uint16_t index = 0u; index < program.output_count; ++index)
        mark_live(program, program.outputs[index].node, live);
    *prepared = prepared_predicate_plan{};
    prepared->version = sequence_program_version;
    prepared->node_count = program.node_count;
    prepared->output_count = program.output_count;
    prepared->semantic_hash = sequence_predicate_program_hash(program);
    prepared->family = predicate_lowering_family::exact_scan;
    for (std::uint16_t index = 0u; index < sequence_program_max_nodes; ++index)
        prepared->original_to_prepared[index] = sequence_program_no_input;
    for (std::uint16_t index = 0u; index < program.node_count; ++index) {
        if (!live[index]) continue;
        prepared->original_to_prepared[index] = prepared->live_node_count++;
        const predicate_instruction& node = program.nodes[index];
        if (node.opcode == predicate_opcode::segment_reduce_request)
            prepared->family = predicate_lowering_family::segment;
        else if (prepared->family != predicate_lowering_family::segment
            && node.opcode != predicate_opcode::exact_motif)
            prepared->family = predicate_lowering_family::fused_plane_mask;
        if (node.opcode == predicate_opcode::shift) {
            if ((node.flags & predicate_shift_right) != 0u)
                prepared->lookahead = prepared->lookahead > node.immediate
                    ? prepared->lookahead : node.immediate;
            else
                prepared->lookbehind = prepared->lookbehind > node.immediate
                    ? prepared->lookbehind : node.immediate;
        } else if (node.opcode == predicate_opcode::spacing_exact
            || node.opcode == predicate_opcode::spacing_within
            || node.opcode == predicate_opcode::run
            || node.opcode == predicate_opcode::any
            || node.opcode == predicate_opcode::all) {
            const std::uint32_t reach = node.immediate - 1u;
            prepared->lookahead = prepared->lookahead > reach
                ? prepared->lookahead : reach;
        }
    }
    prepared->scratch_mask_words_per_window = prepared->live_node_count;
    return predicate_plan_status::ok;
}

} // namespace baseplane::seq
