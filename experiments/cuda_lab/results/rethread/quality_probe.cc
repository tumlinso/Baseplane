#include "lab.hh"
#include <cmath>
#include <iostream>

int main() {
    using namespace bp_cuda_lab;
    const auto input = make_values(1025, 20260929);
    const auto weights = make_weights();
    const auto full = refine_scalar(input, weights, -2.f);
    const auto gated = refine_scalar(input, weights, 0.f);
    unsigned selected = 0, missed_distinct = 0;
    double missed_l1 = 0;
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (select(input[i], 0.f)) { ++selected; continue; }
        double l1 = 0;
        for (int d = 0; d < hidden; ++d)
            l1 += std::abs(double(full[i * hidden + d]) - double(gated[i * hidden + d]));
        missed_l1 += l1;
        if (l1 > 1e-3) ++missed_distinct;
    }
    std::cout << "{\"fixture\":\"seed_20260929_untrained_weights\",\"records\":" << input.size()
              << ",\"selected\":" << selected << ",\"omitted_with_rich_delta_gt_1e-3\":"
              << missed_distinct << ",\"omitted_rich_delta_l1\":" << missed_l1 << "}\n";
}
