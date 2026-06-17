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

} // namespace baseplane::seq
