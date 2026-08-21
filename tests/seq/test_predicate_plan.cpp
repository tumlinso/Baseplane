#include <Baseplane/dna2.hh>

#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace seq = ::baseplane::seq;

namespace {

void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

static_assert(std::is_trivially_copyable_v<seq::sequence_predicate_program>);
static_assert(std::is_standard_layout_v<seq::sequence_predicate_program>);
static_assert(std::is_trivially_copyable_v<seq::prepared_predicate_plan>);
static_assert(sizeof(seq::sequence_event) == 8u);
static_assert(seq::sequence_predicate_abi_version == 1u,
    "sequence predicate ABI version changed unexpectedly");

seq::predicate_instruction instruction(
    seq::predicate_opcode opcode,
    std::uint16_t input_a = seq::sequence_program_no_input,
    std::uint16_t input_b = seq::sequence_program_no_input,
    std::uint32_t immediate = 0u,
    std::uint8_t flags = 0u) {
    return seq::predicate_instruction{
        opcode, seq::predicate_value_kind::mask, flags, 0u,
        input_a, input_b, immediate, 0u};
}

seq::sequence_predicate_program make_program() {
    seq::sequence_predicate_program program{};
    program.version = seq::sequence_program_version;
    program.node_count = 6u;
    program.output_count = 2u;
    program.nodes[0] = instruction(seq::predicate_opcode::base_equal,
        seq::sequence_program_no_input, seq::sequence_program_no_input, 2u);
    program.nodes[1] = instruction(seq::predicate_opcode::gc_mask);
    program.nodes[2] = instruction(seq::predicate_opcode::bit_and, 0u, 1u);
    program.nodes[3] = instruction(
        seq::predicate_opcode::shift, 2u, seq::sequence_program_no_input, 4u,
        seq::predicate_shift_right);
    program.nodes[4] = instruction(seq::predicate_opcode::spacing_within, 2u, 3u, 8u);
    program.nodes[5] = instruction(seq::predicate_opcode::base_equal,
        seq::sequence_program_no_input, seq::sequence_program_no_input, 0u);
    program.outputs[0] = seq::predicate_output{
        4u, 17u, seq::sequence_output_mode::stable_emit, 3u,
        seq::sequence_event_forward_strand, 0u};
    program.outputs[1] = seq::predicate_output{
        2u, 18u, seq::sequence_output_mode::mask, 0u, 0u, 0u};
    return program;
}

void test_verify_prepare_and_hash() {
    const seq::sequence_predicate_program program = make_program();
    require(seq::verify_sequence_predicate_program(program) == seq::predicate_plan_status::ok,
            "valid program rejected");
    seq::prepared_predicate_plan prepared{};
    require(seq::prepare_sequence_predicate_program(program, &prepared)
                == seq::predicate_plan_status::ok,
            "valid program did not prepare");
    require(prepared.version == seq::sequence_program_version, "prepared version mismatch");
    require(prepared.node_count == 6u && prepared.live_node_count == 5u,
            "dead-node removal metadata mismatch");
    require(prepared.original_to_prepared[5] == seq::sequence_program_no_input,
            "dead node retained");
    require(prepared.lookahead == 7u && prepared.lookbehind == 0u,
            "dependency reach mismatch");
    require(prepared.family == seq::predicate_lowering_family::fused_plane_mask,
            "lowering family mismatch");
    require(prepared.semantic_hash == seq::sequence_predicate_program_hash(program),
            "prepared hash mismatch");

    seq::sequence_predicate_program same = program;
    same.nodes[0].reserved = 99u;
    same.outputs[0].reserved = 88u;
    require(seq::sequence_predicate_program_hash(same)
                == seq::sequence_predicate_program_hash(program),
            "reserved bytes entered semantic hash");
    same.outputs[0].predicate_id = 19u;
    require(seq::sequence_predicate_program_hash(same)
                != seq::sequence_predicate_program_hash(program),
            "semantic output change did not alter hash");
    require(seq::prepare_sequence_predicate_program(program, nullptr)
                == seq::predicate_plan_status::null_output,
            "null prepared output accepted");
}

void test_motifs_and_rejections() {
    seq::sequence_predicate_program program{};
    program.version = seq::sequence_program_version;
    program.node_count = 2u;
    program.output_count = 1u;
    program.allowed_motif_count = 1u;
    const std::uint8_t sets[] = {0x1u, 0x6u, 0x8u};
    require(seq::dna2_make_motif32_allowed(
                sets, 3u, 1u, 9u, seq::dna2_strand_mode::both,
                seq::dna2_palindrome_policy::deduplicate,
                &program.allowed_motifs[0]),
            "test motif normalization failed");
    program.nodes[0] = instruction(seq::predicate_opcode::allowed_motif,
        seq::sequence_program_no_input, seq::sequence_program_no_input, 0u);
    program.nodes[1] = instruction(seq::predicate_opcode::run, 0u,
        seq::sequence_program_no_input, 3u);
    program.outputs[0] = seq::predicate_output{
        1u, 9u, seq::sequence_output_mode::count, 0u, 0u, 0u};
    require(seq::verify_sequence_predicate_program(program) == seq::predicate_plan_status::ok,
            "normalized allowed motif program rejected");

    seq::sequence_predicate_program invalid = program;
    invalid.version = 2u;
    require(seq::verify_sequence_predicate_program(invalid)
                == seq::predicate_plan_status::unsupported_version,
            "unknown version accepted");
    invalid = program;
    invalid.nodes[1].input_a = 1u;
    require(seq::verify_sequence_predicate_program(invalid)
                == seq::predicate_plan_status::invalid_input,
            "forward/self reference accepted");
    invalid = program;
    invalid.nodes[1].immediate = 33u;
    require(seq::verify_sequence_predicate_program(invalid)
                == seq::predicate_plan_status::invalid_immediate,
            "oversized span accepted");
    invalid = program;
    invalid.allowed_motifs[0].active_mask = 0u;
    require(seq::verify_sequence_predicate_program(invalid)
                == seq::predicate_plan_status::invalid_motif,
            "forged motif accepted");
    invalid = program;
    invalid.outputs[0].node = 2u;
    require(seq::verify_sequence_predicate_program(invalid)
                == seq::predicate_plan_status::invalid_output,
            "out-of-range output accepted");
    invalid = program;
    invalid.node_count = 0u;
    require(seq::verify_sequence_predicate_program(invalid)
                == seq::predicate_plan_status::invalid_count,
            "empty program accepted");
}

} // namespace

int main() {
    test_verify_prepare_and_hash();
    test_motifs_and_rejections();
    return 0;
}
