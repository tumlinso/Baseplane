# Sequence-grounded effect machines — BP-MOON-030

Four distinct experimental computations preserve their different information contracts.
They use small untrained sequence fixtures. Shared numerical composition belongs to
Cellerator; this family supplies sequence interpretation, witnesses and grammar semantics.

| Card | Mechanism and entry point | Information retained and revisit |
|---|---|---|
| E05 | `EffectHierarchy` in `automata.hpp`: balanced source-derived DFA effect tree using `ce_moon::compose` | Every entry state's final last-two-base state, plus local region bounds. Counts and event history are absent; recover those from exact source. |
| E06 | `summarize_motif`, `concatenate(WitnessedEffect)`, `replay_motif` | Entry-dependent AC counts and first/last C endpoints. Full intermediate events require source replay; witnesses alone cannot explain every output. |
| E07 | `grammar_tile`, `concatenate(GrammarTile)`, `grammar_replay` | Net depth, prefix extrema and unmatched paired-base boundary stacks. Synthetic A...T/C...G syntax; overflow or unknown input explicitly requires replay. |
| E08 | `summarize_alternatives` in `weighted.hpp`, using `ce_moon::effects` | Two possible interpretations with probability, max-plus or Boolean composition. Explicit pruning loses alternatives and cannot be called exact. |

E05/E06 normalize lowercase canonical bases; invalid symbols reset the DFA or motif
prefix. E07 unknown symbols do not establish grammar acceptance. E08 invalid symbols
reset both interpretations to entry state zero; the reset operation is explicit, and
invalid packed payload is never used as a valid base.

Source support uses local half-open bounds into the original `PackedFixture`. The caller
keeps the exact source and its `SourceMap` with source ID, contig, origin, version and
strand. E06 replay maps endpoints through `SourceMap::coordinate`, including reverse
strand. Reverse strand here maps the supplied sequence's coordinates; it does not
silently reverse-complement the supplied sequence. First/last witnesses follow supplied
sequence order. These supports establish provenance, not a lossless learned embedding.

DFA construction is linear in source length with 32 possible entry states. E05 materializes
a binary tree; it queries any entry's final state at the root without rescanning the source.
E06 summary is fixed in entry-state count, with checked 64-bit count and witness-span
addition. Replay accepts caller-owned output and reports produced/stored/dropped counts;
zero capacity is valid. E07 keeps at most the requested boundary capacity in each output;
composition may use twice that capacity temporarily. Its scalar full replay uses source-
length stack storage and never pretends unbounded parsing fits in a constant-size summary.
E08 has two states and a fixed four-edge matrix per tile. These are accounting descriptions,
not timings or speed claims. General semiring arithmetic and pruning are Cellerator-owned.

## Validation and reproduction

```sh
cmake -S experiments/moonshot -B /tmp/bp-moon-automata-build \
  -DCE_MOON_SOURCE_DIR=/path/to/Cellerator/experiments/baseplane_moonshot \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/bp-moon-automata-build \
  --target bp_moon_automata_smoke bp_moon_automata_weighted_smoke -j 1
ctest --test-dir /tmp/bp-moon-automata-build \
  -R '^bp_moon_automata.*smoke$' --output-on-failure
```

`smoke.cpp` checks all 32 DFA entry states against direct scanning, identity, order and
associativity. E06 checks state-dependent counts, cross-seam witnesses, reset validity,
capacity accounting, reverse coordinates and overflow. E06/E07 compare 3,906 source
fixtures of lengths zero through five over A/C/G/T/N at every seam against independent
scalar event/stack oracles. E07 additionally checks boundary mismatches and overflow
replay. `weighted_smoke.cpp` checks merging paths: probability gives 0.42, max-plus gives
best-path probability 0.36, and Boolean gives reachability. Pruning the locally weaker
0.4 edge before the next base changes the eventual probability from 0.42 to 0.06.

The E08 target requires `Cellerator::moonshot_effects`; configuration states explicitly
when that provider is unavailable. E05-E07 remain independently buildable. Exact commands,
source revisions and actual outcomes are recorded in `receipt.json`.

No CUDA kernel was authored in this family, compiled or launched. E05's state table is a
candidate for lane-as-state shuffle composition; E06 counts/witnesses could use state-
dependent lookup; E07 could use packed bounded stacks. Those executor adaptations remain
later work. No performance measurement, biological efficacy, calibrated probability or
trained hierarchy is claimed. All four cards are representative scalar experiments.

The opted-in C++ context configuration indexes production/test paths and excludes this
new experimental directory by omission. Canonical source inspection and this target's
local compilation database provide the scoped fallback; generated indexes were preserved.
