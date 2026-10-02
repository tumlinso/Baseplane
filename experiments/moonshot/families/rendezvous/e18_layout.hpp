#pragma once
#include <cstdint>
namespace bp_moon::discovery::gpu {
constexpr std::uint32_t absent=0xffffffffu;
struct Slot {std::uint64_t key;std::uint32_t width,group;};
struct Row {Slot slots[32];std::uint32_t next;};
struct Query {std::uint64_t key;std::uint32_t width,bucket;};
} // namespace bp_moon::discovery::gpu
