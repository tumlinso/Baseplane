#include <Baseplane/seq/dna2_validity.hh>
#include <Baseplane/seq/sequence_event.hh>
#include <stdexcept>

namespace seq = baseplane::seq;
static void require(bool value) {
    if (!value) throw std::runtime_error("sequence adoption contract");
}
int main() {
    // Invalid packed payload and the 33-base tail remain explicit.
    const std::uint64_t packed[] = {0, 0};
    const std::uint64_t validity[] = {~std::uint64_t{0} ^ (std::uint64_t{1} << 31)};
    const seq::dna2_packed64_valid_view view{{packed, 33, 2}, validity, 1};
    require(seq::dna2_valid_view(view));
    require(!seq::dna2_base_is_valid(view, 31));
    require(seq::dna2_base_is_valid(view, 32));
    require(!seq::dna2_base_is_valid(view, 33));
    require(!seq::dna2_window_is_valid(view, 30, 3));
    const seq::dna2_chunk_coordinates chunk{{17, 4, 2}, 500, 33, 1, 32, 1, 1};
    std::uint64_t coordinate = 0;
    require(seq::dna2_valid_chunk_coordinates(chunk));
    require(seq::dna2_local_to_global(chunk, 31, &coordinate) && coordinate == 531);
    require(!seq::dna2_anchor_is_owned(chunk, 32));
    const seq::sequence_event event{31, 7, 0, seq::sequence_event_reverse_strand};
    const auto roundtrip = seq::sequence_event_from_motif_hit(seq::motif_hit_from_sequence_event(event));
    require(roundtrip.local_position == 31 && roundtrip.flags == seq::sequence_event_reverse_strand);
    const seq::sequence_emit_counts counts{3, 1, 2, 3};
    require(seq::sequence_emit_counts_valid(seq::sequence_output_mode::stable_emit, 1, counts));
}
