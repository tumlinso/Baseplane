#include "lab.hh"
#include <algorithm>
#include <cmath>
#include <iostream>

int main() {
    using namespace bp_cuda_lab;
    Planes ordered{}, rotated{};
    ordered.valid = rotated.valid = 0xffffffffu;
    auto set_pattern = [](Planes& p, unsigned i, unsigned pattern) {
        if (pattern & 4u) p.a |= 1u << i;
        if (pattern & 2u) p.b |= 1u << i;
        if (pattern & 1u) p.c |= 1u << i;
    };
    for (unsigned i = 0; i < 32; ++i) {
        set_pattern(ordered, i, i % 8u);
        set_pattern(rotated, i, (i + 1u) % 8u);
    }
    auto weights = make_weights();
    Vec a = lift_scalar(ordered, weights), b = lift_scalar(rotated, weights);
    double max_embedding_difference = 0;
    for (int d = 0; d < channels; ++d)
        max_embedding_difference = std::max(max_embedding_difference,
                                            std::abs(double(a.x[d]) - b.x[d]));
    // Thirty-two positive lower records of 0.1 and 0.9 share threshold planes.
    // Their means differ by 0.8 despite identical lifted histograms.
    std::cout << "{\"order_sensitive_first_pattern\":[0,1],"
              << "\"rotated_histogram_embedding_max_difference\":" << max_embedding_difference
              << ",\"same_sign_lower_float_means\":[0.1,0.9],"
              << "\"same_sign_magnitude_difference\":0.8}\n";
}
