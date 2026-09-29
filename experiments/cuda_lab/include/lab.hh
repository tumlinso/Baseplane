#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <vector>
#include <stdexcept>

// Experimental, private value types. No Baseplane/Cellerator ABI is replaced.
#ifdef __CUDACC__
#define LAB_HD __host__ __device__
#else
#define LAB_HD
#endif
namespace bp_cuda_lab {
constexpr int warp_width = 32;
constexpr int channels = 4;
constexpr int hidden = 32;
struct Vec { float x[channels]{}; };
struct Planes { std::uint32_t a{}, b{}, c{}, valid{}; };
struct Affine { Vec a{}, b{}; };
struct Match { Vec mean{}; std::uint32_t peers{}, leader{}; };
struct Weights { float lift[channels * 8]{}; float w1[hidden * channels]{};
                 float b1[hidden]{}; float w2[hidden * hidden]{}; };
LAB_HD inline float activate(float x) { return x / (1.f + fabsf(x)); }
LAB_HD inline Affine identity() {
    Affine t{}; for(int d=0; d<channels; ++d) t.a.x[d]=1.f; return t;
}
// Compose earlier L, then later R. Associative over reals, NOT commutative.
LAB_HD inline Affine compose(const Affine& l, const Affine& r) {
    Affine t{};
    for(int d=0; d<channels; ++d) {
        t.a.x[d]=r.a.x[d]*l.a.x[d];
        t.b.x[d]=fmaf(r.a.x[d],l.b.x[d],r.b.x[d]);
    }
    return t;
}
LAB_HD inline bool select(const Vec& v,float threshold) {
    return fmaf(.5f,v.x[1],v.x[0]) > threshold;
}
LAB_HD inline float cheap(const Vec& v,int d) {return v.x[d%channels];}
Weights make_weights();
std::vector<Vec> make_values(std::size_t n, std::uint32_t seed=20260928);
std::vector<Planes> make_planes(std::size_t n, std::uint32_t seed=17);
std::vector<Affine> make_transforms(const std::vector<Vec>& input, float boundary_threshold);
Vec lift_scalar(const Planes&,const Weights&);
Vec lift_bitset(const Planes&,const Weights&);
std::vector<Affine> scan_scalar(const std::vector<Affine>&);
std::vector<Affine> scan_tree_reference(const std::vector<Affine>&);
std::vector<float> refine_scalar(const std::vector<Vec>&,const Weights&,float threshold);
std::vector<float> refine_compacted_reference(const std::vector<Vec>&,const Weights&,float threshold);
std::vector<Match> match_scalar(const std::vector<Vec>&,const std::vector<std::uint32_t>&);
std::vector<Match> match_mask_reference(const std::vector<Vec>&,const std::vector<std::uint32_t>&);
std::uint32_t route_key(const Vec&);
std::uint32_t low_mask(unsigned n);
} // namespace bp_cuda_lab
#undef LAB_HD
