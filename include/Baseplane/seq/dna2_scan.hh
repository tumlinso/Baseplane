#pragma once

#include <Baseplane/core/status.hh>
#include <Baseplane/seq/dna2_motif.hh>
#include <Baseplane/seq/dna2_views.hh>

#include <cstdint>

namespace baseplane::seq {

baseplane::status scan_exact_count_cpu(
    dna2_packed64_view sequence,
    motif32_exact motif,
    std::uint64_t* host_count);

baseplane::status scan_exact_emit_cpu(
    dna2_packed64_view sequence,
    motif32_exact motif,
    compact_motif_hit_buffer host_hits);

baseplane::status dna2_to_planes32_stream_cpu(
    dna2_packed64_view sequence,
    dna2_planes32_stream_mutable_view output);

baseplane::status planes32_stream_base_mask_cpu(
    dna2_planes32_stream_view stream,
    std::uint8_t base_code,
    dna2_mask32_stream_mutable_view output);

baseplane::status planes32_stream_gc_mask_cpu(
    dna2_planes32_stream_view stream,
    dna2_mask32_stream_mutable_view output);

baseplane::status planes32_stream_cpg_start_mask_cpu(
    dna2_planes32_stream_view stream,
    dna2_mask32_stream_mutable_view output);

} // namespace baseplane::seq
