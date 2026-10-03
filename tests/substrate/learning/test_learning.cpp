#include <Baseplane/learning/sequence_routes.hh>
#include <iostream>

namespace learning = baseplane::learning;
static void require(bool value, const char* message) { if (!value) throw std::runtime_error(message); }
template<class F> static void rejects(F call) {
    bool rejected = false;
    try { call(); } catch (const std::exception&) { rejected = true; }
    require(rejected, "expected rejection");
}
static learning::grounded_sequence source(std::string text, std::uint32_t halo = 0) {
    const auto size = static_cast<std::uint32_t>(text.size());
    return {std::move(text), {{17, 4, 2}, 1000, size, halo, size - halo, halo, halo}, 3, 5};
}
static bool scalar_target(const std::string& text, std::size_t position) {
    const char left = text[position - 1], center = text[position], right = text[position + 1];
    return ((center == 'C' || center == 'G') && left == 'A') || right == 'T';
}
static double observed_loss(const learning::route_model& model) {
    double loss = 0;
    for (unsigned row = 0; row < 8; ++row) if (model.observed[row]) {
        const auto p = model.teacher.predict(row);
        loss -= learning::task(row) ? std::log(p) : std::log(1 - p);
    }
    return loss;
}
static void learned_and_constructed() {
    std::array<bool, 8> all{}; all.fill(true);
    const auto model = learning::fit(all, 11, 12, 13, 14, 200);
    require(observed_loss(model) < .1 && observed_loss(model) < 8 * std::log(2.), "actual fitting loss reduction");
    std::size_t classified = 0;
    for (unsigned row = 0; row < 8; ++row) {
        const auto text = learning::example(row);
        const auto input = source(text);
        const auto constructed = learning::encode(input, model, learning::route_kind::constructed);
        for (const auto kind : {learning::route_kind::soft_teacher, learning::route_kind::discrete,
             learning::route_kind::hardened, learning::route_kind::residual}) {
            const auto representation = learning::encode(input, model, kind);
            require(representation.gates.size() == 1 && representation.gates[0].row == row
                && representation.gates[0].selected == scalar_target(text, 1)
                && representation.gates[0].selected == constructed.gates[0].selected,
                "same representation interface as constructed alternative");
            require(representation.gates[0].coordinate == 1001 && representation.value_generation == 5
                && representation.model.teacher == 11 && representation.model.discrete == 12
                && representation.model.hardened == 13 && representation.model.residual == 14,
                "source/artifact provenance");
        }
        ++classified;
    }
    const auto input = source("AACGTANACGTAAAAAAAAAAACGT", 1);
    const auto constructed = learning::encode(input, model, learning::route_kind::constructed, 2, 5);
    const auto learned = learning::encode(input, model, learning::route_kind::hardened, 2, 5);
    require(learned.gates.size() == constructed.gates.size()
        && learned.chunks.size() == constructed.chunks.size(), "held-out sequence layout");
    for (std::size_t i = 0; i < learned.gates.size(); ++i)
        require(learned.gates[i].selected == constructed.gates[i].selected
            && learned.gates[i].selected == scalar_target(input.exact, learned.gates[i].anchor),
            "held-out sequence source oracle");
    std::uint32_t end = 0;
    std::set<std::uint32_t> lengths;
    for (const auto chunk : learned.chunks) {
        require(chunk.begin == end && chunk.end > chunk.begin && chunk.end - chunk.begin <= 5,
            "quiet/invalid region chunk coverage");
        lengths.insert(chunk.end - chunk.begin); end = chunk.end;
    }
    require(end == input.exact.size() && lengths.size() > 1, "nonuniform capped chunks");
    require(learned.inspected_anchors == input.exact.size() && learned.invalid_contexts >= 5,
        "invalid context must be explicit");
    require(learning::gradients(learning::route_kind::soft_teacher) == learning::gradient_convention::ce_bce_logits
        && learning::gradients(learning::route_kind::hardened) == learning::gradient_convention::stop_gradient,
        "gradient convention");
    std::cout << "observed_rows=" << classified << " teacher_loss=" << observed_loss(model)
        << " heldout_sequence_gates=" << learned.gates.size() << '\n';
}
static void residual_refinement() {
    const std::array<bool, 8> observed{true, true, true, true, false, false, false, false};
    const auto partial = learning::fit(observed, 21, 22, 23, 24);
    const auto evidence = source(learning::example(4)); // Unobserved quiet row, actual model failure.
    const auto before = learning::encode(evidence, partial, learning::route_kind::hardened);
    require(before.gates.size() == 1 && before.gates[0].selected && !scalar_target(evidence.exact, 1),
        "declared heldout counterexample");
    const auto repaired = learning::refine(partial, {evidence}, 25);
    const auto after = learning::encode(evidence, repaired.model, learning::route_kind::residual);
    require(repaired.failures.size() == 1 && repaired.failures[0].row == 4
        && repaired.failures[0].coordinate == 1001 && !after.gates[0].selected,
        "source-grounded residual correction");
    require(repaired.model.teacher.logits == partial.teacher.logits
        && repaired.model.discrete.version == partial.discrete.version
        && repaired.model.hardened.circuit_version == partial.hardened.circuit_version
        && repaired.model.residual.version == 25, "independent immutable artifact revisions");
    require(learning::encode(evidence, partial, learning::route_kind::hardened).gates[0].selected,
        "refinement changed old hardened route");
    auto stale = repaired.model; ++stale.residual.hardened_version;
    rejects([&] { learning::encode(evidence, stale, learning::route_kind::residual); });
    stale = repaired.model; ++stale.discrete.teacher_version;
    rejects([&] { learning::encode(evidence, stale, learning::route_kind::discrete); });
    rejects([&] { learning::refine(partial, {evidence}, 24); });
    std::cout << "actual_counterexamples=" << repaired.failures.size() << " residual_version=25\n";
}
static void quiet_service() {
    std::array<bool, 8> all{}; all.fill(true);
    const auto model = learning::fit(all, 31, 32, 33, 34);
    const auto quiet = source(std::string(17, 'A'));
    const auto encoded = learning::encode(quiet, model, learning::route_kind::hardened);
    for (const auto& gate : encoded.gates) require(!gate.selected, "quiet fixture must be unselected");
    std::uint32_t cursor = 0; std::set<std::uint32_t> visited;
    for (unsigned epoch = 0; epoch < quiet.exact.size(); ++epoch) {
        const auto wave = learning::schedule(encoded, quiet.exact.size(), 1, cursor);
        require(wave.size() == 1 && wave[0].reason == learning::service_reason::exploration, "reserved exploration");
        visited.insert(wave[0].position);
    }
    require(visited.size() == quiet.exact.size(), "quiet positions permanently starved");
    require(learning::schedule(encoded, quiet.exact.size(), 0, cursor).empty(), "zero budget");
    rejects([&] { learning::schedule(encoded, 1, 1, cursor); });
    const auto mixed = learning::encode(source("AACGTANACGT"), model, learning::route_kind::soft_teacher);
    const auto wave = learning::schedule(mixed, mixed.coordinates.base_count, 3, cursor);
    require(wave.size() <= 3 && wave[0].reason == learning::service_reason::exploration, "capacity-bounded priorities");
    std::cout << "quiet_positions_served=" << visited.size() << " positive_budget_epochs=17\n";
}
int main() {
    learned_and_constructed(); residual_refinement(); quiet_service();
    std::cout << "declared learned triplet task and versioned routes passed\n";
}
