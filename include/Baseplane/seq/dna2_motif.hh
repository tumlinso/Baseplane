#pragma once

#include <Baseplane/seq/dna2_ops.hh>

#include <cstdint>

#if defined(__CUDACC__)
#define BASEPLANE_DNA2_MOTIF_HD __host__ __device__ __forceinline__
#else
#define BASEPLANE_DNA2_MOTIF_HD inline
#endif

namespace baseplane::seq {

struct motif32_exact {
    std::uint64_t packed;
    std::uint64_t active_fields;
    std::uint8_t length;
    std::uint8_t max_mismatches;
    std::uint16_t motif_id;
};

struct motif_hit {
    std::uint32_t position;
    std::uint16_t motif_id;
    std::uint8_t mismatches;
    std::uint8_t strand;
};

struct compact_motif_hit_buffer {
    motif_hit* hits;
    std::uint32_t capacity;
    std::uint32_t* records_written;
    std::uint32_t* records_dropped;
};

BASEPLANE_DNA2_MOTIF_HD motif32_exact make_motif32_exact(
    dna2_word64 motif,
    std::uint8_t length,
    std::uint8_t max_mismatches,
    std::uint16_t motif_id = 0u) {
    const std::uint32_t active_mask = detail::active_mask_from_length(length);
    return motif32_exact{
        motif.packed,
        detail::spread_active_mask_to_packed_fields(active_mask),
        length,
        max_mismatches,
        motif_id
    };
}

BASEPLANE_DNA2_MOTIF_HD dna2_word64 motif_word(motif32_exact motif) {
    return dna2_word64{motif.packed};
}

} // namespace baseplane::seq

#undef BASEPLANE_DNA2_MOTIF_HD
