# BP-MOON-100 receipt

Observed 2 October 2026. E33, E34, E35 and E36 are authored, compiled, run and compared experimentally on CPU. Root supplied the active claim and bounded source handle `wfc_qBqhI29r8XzJ0SbdSnhpM7dJt5vS4tKLUmF7iHba68E` before work. This worker owns only `experiments/moonshot/families/incremental`; root retains lifecycle and acceptance.

Working directory:
`/home/tumlinson/Baseplane/.git/todo-orchestrator/1a7de0f4-347d-4707-9fc3-5f89d7c4d6b3/workflow-workspaces/bp-moon-l-incremental`

Actual commands:

```sh
cmake -S experiments/moonshot -B /tmp/bp-moon-incremental-build \
  -DCE_MOON_SOURCE_DIR=/home/tumlinson/Cellerator/experiments/baseplane_moonshot \
  -DBUILD_TESTING=ON
python /tmp/moonshot_build_slot.py cmake --build /tmp/bp-moon-incremental-build --target bp_moon_incremental_smoke -j1
ctest --test-dir /tmp/bp-moon-incremental-build -R '^bp_moon_incremental_smoke$' --output-on-failure
/tmp/bp-moon-incremental-build/families/incremental/bp_moon_incremental_smoke > experiments/moonshot/families/incremental/semantic-output.txt
```

GNU C++ 13.3.0 compiled the target. The focused CTest passed 1/1. `semantic-output.txt` contains exact observed counters. Provider HEAD observed after the final tests was `217def8f85a98ff131317476eb1288a98209c3b1`; the consumed Cellerator `include/ce_moon/reference.hpp` SHA256 is `dcb8b316a823c48fc7ffcbbee0cd7ab7d9bf06e3935d5fcdf972a751269138ed`, matching the foundation implementation. E33 and E36 consume its DFA operations through the existing Baseplane sequence adapters. No new numerical backend or provider interface was introduced.

The first build reported that `ce_moon::dirty_closure` was absent from the adopted provider. E34 was corrected to use family-local sequence dependency bookkeeping with a visited frontier; this is an explicit source/directory invalidation fixture, without introducing a generic numerical or runtime framework. The corrected target compiled and all semantic checks passed.

E33 checks forced hash collisions, exact content ID sharing, packed-equal invalid payload distinction, independent context/state versions, reverse occurrence provenance and coordinate overflow rejection. E34 checks dirty ancestors, both value and membership dependencies, old posting removal/new posting insertion, independent sibling preservation, stale source-version rejection and cyclic fixed-point termination. E35 checks outside-support reuse, edits to negative inspected evidence, query/weight/cell-state/representation changes, missing and skipped edit histories, insertion-induced shifts, reverse-coordinate shifts, changed origin and plan-versus-result behavior. E36 compares static and incremental exact effects, multi-level node reuse, exact token expansion including invalid bases, ordered pair identity, distinct occurrence coordinates and invalid empty-span rejection.

The family stores original source strings, owning fixture vectors/maps, exact grammar nodes and occurrence descriptors. It does not claim an efficient compressed format, constant-time variants, global biological answers or learned residuals. No GPU launch, CUDA build, benchmark, sanitizer, large dataset, persistent cache or production integration test was run. Generated Todo projections and unrelated work were preserved. See `README.md` for source-support, loss/revisit and buffer/counter limits.
