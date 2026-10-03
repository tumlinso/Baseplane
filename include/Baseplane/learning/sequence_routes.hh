#pragma once
#include <Baseplane/seq/dna2_validity.hh>
#include <learning.hpp>
#include <array>
#include <functional>
#include <set>

namespace baseplane::learning {
namespace numeric = ce_moon::learning;
using u64 = std::uint64_t;
// Version 1 declares a synthetic sequence-local task, not biological function:
// valid triplet -> (center GC and previous A) or next T/U.
inline bool task(unsigned row) {
    if (row >= 8) throw std::out_of_range("task row");
    return ((row & 4) && (row & 2)) || (row & 1);
}
struct grounded_sequence {
    std::string exact;
    seq::dna2_chunk_coordinates coordinates;
    u64 structure_epoch, value_generation;
    void validate() const {
        if (!seq::dna2_valid_chunk_coordinates(coordinates) || exact.size() != coordinates.base_count)
            throw std::invalid_argument("learned source");
    }
};
struct features { unsigned row = 0; bool valid = false, owned = false; };
inline features triplet(const grounded_sequence& source, std::uint32_t anchor) {
    if (!anchor || anchor >= source.exact.size() || anchor + 1 >= source.exact.size()) return {};
    const auto left = seq::dna2_encode_base_with_validity(source.exact[anchor - 1]);
    const auto center = seq::dna2_encode_base_with_validity(source.exact[anchor]);
    const auto right = seq::dna2_encode_base_with_validity(source.exact[anchor + 1]);
    if (!left.valid || !center.valid || !right.valid) return {};
    const bool gc = center.code == 1 || center.code == 2;
    return {unsigned(4 * gc + 2 * (left.code == 0) + (right.code == 3)), true,
        seq::dna2_anchor_is_owned(source.coordinates, anchor)};
}
inline std::string example(unsigned row) {
    if (row >= 8) throw std::out_of_range("example row");
    return std::string{(row & 2) ? 'A' : 'C', (row & 4) ? 'C' : 'A', (row & 1) ? 'T' : 'G'};
}
struct discrete_route { u64 teacher_version, version; double threshold = .5; };
struct residual_route {
    u64 hardened_version, version, task_version;
    std::uint8_t corrected_rows = 0, targets = 0;
};
struct route_model {
    u64 task_version = 1;
    numeric::TruthTeacher teacher;
    discrete_route discrete;
    numeric::HardLut hardened;
    residual_route residual;
    std::array<bool, 8> observed{};
    void validate() const {
        if (task_version != 1 || !teacher.weight_version || !discrete.version || !hardened.circuit_version
            || !residual.version || discrete.teacher_version != teacher.weight_version
            || hardened.teacher_version != teacher.weight_version
            || residual.hardened_version != hardened.circuit_version || residual.task_version != task_version
            || !std::isfinite(discrete.threshold) || discrete.threshold < 0 || discrete.threshold > 1)
            throw std::invalid_argument("learned artifact versions");
        for (unsigned row = 0; row < 8; ++row) teacher.predict(row);
    }
};
// Fitting stays in the existing CE numerical provider. Unobserved rows have
// explicit neutral .5 targets; their zero logits receive zero fitting gradient.
inline route_model fit(std::array<bool, 8> observed, u64 teacher_version, u64 discrete_version,
                       u64 hardened_version, u64 residual_version, std::size_t epochs = 200) {
    std::array<double, 8> targets{};
    for (unsigned row = 0; row < 8; ++row) {
        const auto text = example(row);
        const grounded_sequence sample{text, {{1, 0, row}, 0, 3, 0, 3, 0, 0}, 1, 1};
        const auto feature = triplet(sample, 1);
        if (!feature.valid || feature.row != row) throw std::logic_error("sequence training row");
        targets[row] = observed[row] ? double(task(feature.row)) : .5;
    }
    const auto teacher = numeric::fit_truth_table(targets, epochs, .5, teacher_version);
    route_model model{1, teacher, {teacher_version, discrete_version, .5},
        numeric::harden(teacher, hardened_version), {hardened_version, residual_version, 1, 0, 0}, observed};
    model.validate(); return model;
}
enum class route_kind { constructed, soft_teacher, discrete, hardened, residual };
enum class gradient_convention { exact_discrete_no_gradient, ce_bce_logits, stop_gradient };
inline gradient_convention gradients(route_kind kind) {
    if (kind == route_kind::constructed) return gradient_convention::exact_discrete_no_gradient;
    if (kind == route_kind::soft_teacher) return gradient_convention::ce_bce_logits;
    if (kind == route_kind::discrete || kind == route_kind::hardened || kind == route_kind::residual)
        return gradient_convention::stop_gradient;
    throw std::invalid_argument("route kind");
}
struct versions { u64 task, teacher, discrete, hardened, residual; };
struct gate {
    std::uint32_t anchor;
    u64 coordinate;
    unsigned row;
    double probability;
    bool selected;
};
struct chunk { std::uint32_t begin, end; };
struct representation {
    seq::dna2_chunk_coordinates coordinates;
    u64 structure_epoch, value_generation;
    versions model;
    route_kind kind;
    gradient_convention gradient;
    std::vector<gate> gates;
    std::vector<chunk> chunks;
    std::size_t inspected_anchors = 0, invalid_contexts = 0;
};
inline representation encode(const grounded_sequence& source, const route_model& model,
                             route_kind kind, std::uint32_t minimum_chunk = 2,
                             std::uint32_t maximum_chunk = 8) {
    source.validate(); model.validate();
    if (!minimum_chunk || maximum_chunk < minimum_chunk) throw std::invalid_argument("chunk lengths");
    representation output{source.coordinates, source.structure_epoch, source.value_generation,
        {model.task_version, model.teacher.weight_version, model.discrete.version,
         model.hardened.circuit_version, model.residual.version}, kind, gradients(kind), {}, {}, 0, 0};
    std::uint32_t begin = 0;
    for (std::uint32_t anchor = 0; anchor < source.exact.size(); ++anchor) {
        ++output.inspected_anchors;
        const auto feature = triplet(source, anchor);
        bool selected = false;
        if (!feature.valid) ++output.invalid_contexts;
        if (feature.valid && feature.owned) {
            double probability = 0;
            if (kind == route_kind::constructed) probability = double(task(feature.row));
            else if (kind == route_kind::soft_teacher || kind == route_kind::discrete)
                probability = model.teacher.predict(feature.row);
            else {
                selected = model.hardened.predict(feature.row);
                if (kind == route_kind::residual && ((model.residual.corrected_rows >> feature.row) & 1))
                    selected = (model.residual.targets >> feature.row) & 1;
                probability = double(selected);
            }
            if (kind == route_kind::soft_teacher || kind == route_kind::constructed) selected = probability >= .5;
            else if (kind == route_kind::discrete) selected = probability >= model.discrete.threshold;
            u64 coordinate = 0; seq::dna2_local_to_global(source.coordinates, anchor, &coordinate);
            output.gates.push_back({anchor, coordinate, feature.row, probability, selected});
        }
        // Every source position remains in a capped chunk, even when its model
        // score is quiet or its context is invalid.
        if (anchor > begin && (anchor - begin >= maximum_chunk
            || (anchor - begin >= minimum_chunk && selected))) {
            output.chunks.push_back({begin, anchor}); begin = anchor;
        }
    }
    if (begin < source.exact.size()) output.chunks.push_back({begin, static_cast<std::uint32_t>(source.exact.size())});
    return output;
}
struct counterexample {
    seq::dna2_chunk_identity source;
    u64 structure_epoch, value_generation, coordinate;
    unsigned row;
    bool hardened, exact_target;
};
struct refinement { route_model model; std::vector<counterexample> failures; };
inline refinement refine(const route_model& current, const std::vector<grounded_sequence>& sources,
                         u64 residual_version) {
    current.validate();
    if (residual_version <= current.residual.version) throw std::invalid_argument("residual revision");
    refinement result{current, {}}; result.model.residual.version = residual_version;
    for (const auto& source : sources) {
        const auto encoded = encode(source, current, route_kind::hardened);
        for (const auto& gate : encoded.gates) {
            const auto target = task(gate.row);
            if (gate.selected == target) continue;
            u64 coordinate = 0; seq::dna2_local_to_global(source.coordinates, gate.anchor, &coordinate);
            result.failures.push_back({source.coordinates.identity, source.structure_epoch,
                source.value_generation, coordinate, gate.row, gate.selected, target});
            result.model.residual.corrected_rows |= std::uint8_t(1u << gate.row);
            if (target) result.model.residual.targets |= std::uint8_t(1u << gate.row);
            else result.model.residual.targets &= std::uint8_t(~(1u << gate.row));
        }
    }
    result.model.validate(); return result;
}
enum class service_reason { exploration, model_priority };
struct service { std::uint32_t position; service_reason reason; };
// With positive budget, the reserved round-robin item serves every position
// within N calls independently of model score. Zero budget explicitly does no work.
inline std::vector<service> schedule(const representation& representation, std::uint32_t source_length,
                                    std::size_t budget, std::uint32_t& exploration_cursor) {
    std::vector<service> output;
    if (source_length != representation.coordinates.base_count)
        throw std::invalid_argument("service source length");
    if (!source_length || !budget) return output;
    const auto explored = exploration_cursor % source_length;
    output.push_back({explored, service_reason::exploration});
    exploration_cursor = (explored + 1) % source_length;
    auto gates = representation.gates;
    std::stable_sort(gates.begin(), gates.end(), [](const gate& a, const gate& b) {
        return a.probability > b.probability;
    });
    for (const auto& gate : gates) {
        if (gate.anchor >= source_length) throw std::invalid_argument("service source support");
        if (output.size() >= budget) break;
        if (gate.anchor != explored) output.push_back({gate.anchor, service_reason::model_priority});
    }
    return output;
}

} // namespace baseplane::learning
