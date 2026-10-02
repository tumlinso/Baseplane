#pragma once
#include "automata.hpp"
#include <ce_moon/effects.hpp>

namespace bp_moon::automata {
using ce_moon::effects::Algebra;
using WeightedAlternatives = ce_moon::effects::Weighted<2>;
// E08: two synthetic sequence interpretations. A branches, C merges the paths.
// These probabilities are toy parameters, not calibrated biological evidence.
inline WeightedAlternatives alternative_base(char raw, Algebra algebra) {
    const auto base = canonical(raw);
    std::array<std::array<double, 2>, 2> probabilities{};
    switch (base) {
    case 'A': probabilities = {{{0.6, 0.4}, {0.0, 0.8}}}; break;
    case 'C': probabilities = {{{0.1, 0.0}, {0.9, 0.0}}}; break;
    case 'G': probabilities = {{{0.5, 0.5}, {0.5, 0.5}}}; break;
    case 'T': probabilities = {{{1.0, 0.0}, {0.0, 1.0}}}; break;
    default: probabilities = {{{1.0, 0.0}, {1.0, 0.0}}}; break; // Invalid symbol resets, never payload A.
    }
    WeightedAlternatives result;
    result.algebra = algebra;
    for (std::size_t i = 0; i < 2; ++i) {
        for (std::size_t j = 0; j < 2; ++j) {
            const auto probability = probabilities[i][j];
            if (algebra == Algebra::probability) result.weight[i][j] = probability;
            else if (algebra == Algebra::max_plus)
                result.weight[i][j] = probability == 0 ? -std::numeric_limits<double>::infinity() : std::log(probability);
            else result.weight[i][j] = probability != 0;
        }
    }
    result.validate();
    return result;
}
inline WeightedAlternatives summarize_alternatives(std::string_view sequence, Algebra algebra) {
    auto result = WeightedAlternatives::identity(algebra);
    for (char base : sequence) result = ce_moon::effects::compose(result, alternative_base(base, algebra));
    return result;
}
} // namespace bp_moon::automata
