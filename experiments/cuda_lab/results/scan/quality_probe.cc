#include "lab.hh"
#include <algorithm>
#include <cmath>
#include <iostream>

int main() {
    using namespace bp_cuda_lab;
    auto error = [](const Affine& a, const Affine& b) {
        double result = 0;
        for (int d = 0; d < channels; ++d) {
            result = std::max(result, std::abs(double(a.a.x[d]) - b.a.x[d]));
            result = std::max(result, std::abs(double(a.b.x[d]) - b.b.x[d]));
        }
        return result;
    };
    std::cout << "{\"fixture\":\"held_out_seed_20260929\",\"depths\":[";
    bool first = true;
    for (std::size_t n : {33u, 1025u, 65536u}) {
        auto transforms = make_transforms(make_values(n, 20260929), .6f);
        auto direct = scan_scalar(transforms);
        auto tree = scan_tree_reference(transforms);
        double max_error = 0;
        for (std::size_t i = 0; i < n; ++i) max_error = std::max(max_error, error(direct[i], tree[i]));
        if (!first) std::cout << ',';
        first = false;
        std::cout << "{\"n\":" << n << ",\"max_absolute_error\":" << max_error << '}';
    }
    Affine positive = identity(), negative = identity();
    for (int d = 0; d < channels; ++d) {
        positive.a.x[d] = negative.a.x[d] = .5f;
        positive.b.x[d] = 1.f;
        negative.b.x[d] = -1.f;
    }
    auto forward = scan_scalar({positive, negative});
    auto reverse = scan_scalar({negative, positive});
    std::cout << "],\"same_mean_order_witness\":{\"mean_a\":0.5,\"mean_b\":0,"
              << "\"forward_final_b\":" << forward.back().b.x[0]
              << ",\"reverse_final_b\":" << reverse.back().b.x[0] << "}}\n";
}
