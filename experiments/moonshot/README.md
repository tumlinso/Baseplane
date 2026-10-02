# Sequence moonshot foundation

This isolated C++17 substrate grounds experiments in exact sequence, validity and source coordinates. Cellerator owns the general numerical provider from inception. The supplied mixed seed was split by ownership; the immutable planning package remains available for comparison.

```sh
cmake -S experiments/moonshot -B /tmp/bp-moon-foundation \
  -DCE_MOON_SOURCE_DIR=/explicit/path/to/experiments/baseplane_moonshot \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build /tmp/bp-moon-foundation -j 2
ctest --test-dir /tmp/bp-moon-foundation --output-on-failure
/tmp/bp-moon-foundation/bp_moon_demo
```

The provider source path is explicit and has no sibling default. An enclosing build may instead supply `Cellerator::moonshot`; an installed provider may expose the `CelleratorMoonshot` CMake package. `Baseplane::moonshot` exports this directory's headers and consumes that provider. The sequence adapter needs `ce_moon::Dfa32`, `compose` and `ResidualTree` from `ce_moon/reference.hpp`.

`bp_moon/reference.hpp` preserves the seed's sequence fixture, mask/LUT/rank/count helpers, toy sequence-to-state effects, source-support union and exact sequence dictionary. Lowercase canonical bases are valid and share the uppercase transition semantics; original spelling remains recoverable. Invalid packed payloads have cleared validity and reset the toy transition. The last-two-base toy effect loses sequence information; the fixture retains the original sequence for exact recovery.

`bp_moon/source.hpp` attaches source ID, contig, version, strand and origin to fixture coordinates. Reverse maps convert an oriented fixture position to its source coordinate; callers supply the oriented sequence. Emission accepts caller-owned output and capacity, skips invalid payloads, and reports produced/stored/dropped counts. It preserves selection order. Mask counters explicitly report overflow instead of saturating silently. Support union keeps gaps between disjoint source intervals.

The demonstration uses fixed four-base windows, sequence-derived effects and the Cellerator residual tree. Its scalar threshold pruning is meaningful only for that toy query. Exact source revisits, tree visits and dictionary cardinality are reported; no hierarchy-learning, biological-validation or timing claim follows from them.

Each family owns `families/<name>/CMakeLists.txt` and its source directory. Configure-time discovery adds those local targets without editing this shared file; family targets can link `Baseplane::moonshot` and `Cellerator::moonshot`. Shared headers, host targets, CUDA/Python seams and this file retain one integration owner.

CUDA is optional (`BP_MOON_ENABLE_CUDA=OFF` by default). Enabling it sets architecture 70 for later family targets and requires an sm_70-capable toolchain. The foundation introduces no CUDA kernel or launch. General CUDA numerical seeds and Python learning fixtures live with Cellerator. Resource leases, explicit streams and capacity checks remain required for future GPU execution.

The repository ctxpp configuration indexes public source/test paths, excluding this new isolated directory. Canonical source was inspected directly and the isolated build exports its compilation database. Existing ctxpp status reports stale source and prior benchmark parse failures; this foundation does not refresh unrelated indexes or claim a semantic-index pass.
