#include "weighted.hpp"
#include <iostream>

using namespace bp_moon::automata;
static void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}
int main() {
    try {
        const auto probability = summarize_alternatives("AC", Algebra::probability);
        const auto maximum = summarize_alternatives("AC", Algebra::max_plus);
        const auto boolean = summarize_alternatives("AC", Algebra::boolean);
        require(std::abs(probability.weight[0][0] - 0.42) < 1e-12, "E08 probability sums merging paths");
        require(std::abs(std::exp(maximum.weight[0][0]) - 0.36) < 1e-12, "E08 max-plus retains best path");
        require(boolean.weight[0][0] == 1 && boolean.weight[0][1] == 0, "E08 Boolean reachability");
        const auto pruned = ce_moon::effects::prune_below(alternative_base('A', Algebra::probability), 0.5);
        const auto continued = ce_moon::effects::compose(pruned.effect, alternative_base('C', Algebra::probability));
        require(pruned.approximate && pruned.removed == 1 && std::abs(pruned.removed_probability_mass - 0.4) < 1e-12,
                "E08 declared pruning loss");
        require(std::abs(continued.weight[0][0] - 0.06) < 1e-12, "E08 locally weaker path changes later answer");
        const auto reset = summarize_alternatives("AnC", Algebra::probability);
        require(std::abs(reset.weight[0][0] - 0.1) < 1e-12, "E08 invalid symbol reset");
        for (auto algebra : {Algebra::probability, Algebra::max_plus, Algebra::boolean}) {
            const auto a = alternative_base('A', algebra), c = alternative_base('C', algebra), g = alternative_base('G', algebra);
            const auto left = ce_moon::effects::compose(ce_moon::effects::compose(a, c), g);
            const auto right = ce_moon::effects::compose(a, ce_moon::effects::compose(c, g));
            for (std::size_t i = 0; i < 2; ++i) for (std::size_t j = 0; j < 2; ++j)
                require(left.weight[i][j] == right.weight[i][j] || std::abs(left.weight[i][j] - right.weight[i][j]) < 1e-12,
                        "E08 algebra associativity tolerance");
        }
        std::cout << "E08 probability/max-plus/Boolean alternatives; explicit pruning counterexample: PASS\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
