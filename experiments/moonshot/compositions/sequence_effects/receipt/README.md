# C01 — Sequence effects that answer questions

`source/c01.hpp` connects E01, E05, E09 and E25 in one executable path:

1. Exact packed sequence supplies low/high/valid bit planes.
2. The logic circuit computes GC selection as XOR of low/high planes, masked by validity.
3. Those actual predicate bits condition a source fixture: selected C/G retain their canonical symbols; every other position is an explicit reset.
4. The same conditioned fixture builds finite-state and floating affine effect hierarchies.
5. Incoming finite and floating states query the two root effects.
6. Both resulting answers drive the synthetic policy `floating[0] > cutoff + finite`.
7. When requested, the demand family traverses the exact original source for GC supports. Budget/capacity limits return deferred source intervals; `resume` revisits precisely those supports with explicit output accounting.

Predicate conditioning deliberately discards non-GC identity in these response summaries.
Non-GC valid bases and invalid bases share a reset effect, while the original exact source
retains their distinction for refinement. This is a declared query-specific computation,
not an assertion that the GC summary is a lossless genome embedding. Parameters and the
refinement policy are synthetic and untrained.

Finite and floating answers have separate fields and entry-state types. The final demand
output is exact singleton source support, with source ID, contig, version and coordinates.
The source map retains strand; reverse strand maps the supplied sequence's positions and
does not silently reverse-complement it. Mask domains retain source/version/word identity.

The fixture is bounded by the demand family's 4,096-base limit. Both hierarchies materialize
source-derived maps and construction work remains proportional to input. Demand queries
also construct their source certificates; this cost is part of the path. `resume` scans
positions to identify requested support, then reads only requested valid payloads. Its
membership work is bounded by source length times deferred interval count. It deduplicates
overlapping requested ranges by position. Produced/stored/dropped describe known GC
matches from replay; remaining unknown query work stays in deferred support.

The smoke test compares both response results to independent scalar scans. It uses a
33-base fixture with C/G across packed positions 31/32, an invalid neighbor and tail
padding, checking all 32 entry states in forward and reverse maps. It verifies that
changing either incoming state changes the actual refinement decision. Wave-capacity,
work-budget and zero-output limits preserve deferred work, and repeated bounded replay
recovers the unlimited source result. Foreign versions, illegal states and length
mismatches are rejected.

Standalone reproduction:

```sh
cmake -S experiments/moonshot/compositions/sequence_effects -B /tmp/bp-moon-c01-build \
  -DCE_MOON_SOURCE_DIR=/path/to/Cellerator/experiments/baseplane_moonshot \
  -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build /tmp/bp-moon-c01-build --target bp_moon_c01_smoke -j 1
ctest --test-dir /tmp/bp-moon-c01-build -R '^bp_moon_c01_smoke$' --output-on-failure
```

The local CMake also works under the composition aggregate using its existing
`Baseplane::moonshot` target. Numerical composition/application remains in Cellerator.
No CUDA kernel, GPU run, timing claim, trained hierarchy or biological efficacy was added.
Actual commands, provider identity and source hashes are in `evidence.json`.
