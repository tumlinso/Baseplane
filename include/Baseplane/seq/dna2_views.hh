#pragma once

#include <cstdint>

namespace baseplane::seq {

struct dna2_packed64_view {
    const std::uint64_t* words;
    std::uint64_t n_bases;
    std::uint64_t n_words;
};

struct dna2_planes32_stream_view {
    const std::uint32_t* lo_words;
    const std::uint32_t* hi_words;
    std::uint64_t n_words;
};

} // namespace baseplane::seq
