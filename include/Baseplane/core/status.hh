#pragma once

#include <cstdint>

namespace baseplane {

enum class status_code : std::uint8_t {
    ok = 0,
    invalid_argument = 1,
    capacity_exceeded = 2,
    cuda_error = 3,
};

struct status {
    status_code code;
    std::uint32_t detail;
};

inline constexpr status ok_status() {
    return status{status_code::ok, 0u};
}

inline constexpr status invalid_argument_status(std::uint32_t detail = 0u) {
    return status{status_code::invalid_argument, detail};
}

inline constexpr status capacity_exceeded_status(std::uint32_t dropped) {
    return status{status_code::capacity_exceeded, dropped};
}

inline constexpr status cuda_error_status(std::uint32_t cuda_code) {
    return status{status_code::cuda_error, cuda_code};
}

inline constexpr bool is_ok(status s) {
    return s.code == status_code::ok;
}

} // namespace baseplane
