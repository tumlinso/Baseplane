# Sequence response consumers — BP-MOON-040

This family uses the Cellerator CE-MOON-020 provider for four distinct continuous
response experiments. `response.hpp` owns the toy sequence vocabulary, exact source
support and consumer queries. All general operator composition, application, jets and
checkpoint numerics come from `ce_moon/reference.hpp` and `ce_moon/effects.hpp`.

| Card | Representative mechanism | Semantic boundary |
|---|---|---|
| E09 | `ResponseHierarchy`: source-derived two-channel permutation/diagonal affine effects in a balanced region tree | Routes channels but each output reads one input channel. The summary supports any finite entry state; it omits internal trajectory. |
| E10 | `BlockRegion`: source-derived two blocks of two channels with dense within-block mixing | A channel can read both members of its selected block. Fixed block routing remains closed; arbitrary dense channel coupling does not. |
| E11 | `JetRegion`: compose local quadratic jets of source-derived nonlinear responses; refine/reanchor outside radius | Composition truncates third and higher orders. Radius is a query policy, with no certified error bound. Exact scalar source replay supplies omitted terms. |
| E12 | `SequenceCheckpoints`: separate forward and reverse-order source recurrences, reconstructed at requested boundaries | Right context follows reversed source order. Forward suffix response is a separate query, with different operational order. Neither inverts the other. |

All canonical bases accept lowercase. Unknown symbols reset the numerical state to zero;
invalid packed payload never supplies a canonical base. E09 uses C for channel swap;
E10 uses C for block swap and explicit small dense matrices; E11 maps A/C/G to quadratic
curvatures 0.1/0.2/-0.05. These untrained parameters define synthetic computations and
carry no biological or calibration claim.

E09 stores local half-open source bounds per tree node and a `SourceMap`. E10-E12 retain
the source ID, contig, origin, strand, version and length. E11 owns its exact source copy
for refinement; E09/E10 consumers keep the original source alongside the response map.
These maps establish provenance. They do not establish that an embedding preserves all
sequence information. Reverse strand changes coordinate mapping; the right-context
experiment reverses the supplied sequence order without reverse-complementing it.

## Costs and loss

Construction inspects all source bases. E09 retains roughly two nodes per base, each with
a two-channel affine map. E10 retains one composed fixed-size block map per supplied
region; source revisit is required for intermediate states or different interpretations.
Monomial/block closure is algebraic; binary64 rounding depends on composition order.
The smoke comparisons use a relative tolerance, with no universal numerical error claim.

E11 stores a scalar value/linear/quadratic response around an explicit center. An in-radius
query performs no source replay. For source AC at center zero, query 0.1 omits a measured
4.02e-5 in cubic/quartic terms. Query 1 exceeds radius, returns source replay value 1.342
and reanchors the cache at 1. Refinement performs two source traversals: one exact replay
and one jet reconstruction. The returned counters report those traversals separately.
The current implementation keeps its original untrained radius policy after reanchoring.

E12 retains per-base effects and coarse prefix/suffix maps for both directions. This is
O(N) construction and source-effect storage, not a claim of compressed genome storage.
Queries reconstruct less than the checkpoint stride in either direction. The nine-base
fixture at stride three retains sixteen maps across both provider objects. Local replay
is numerical effect replay; exact sequence remains available for changed semantics.

No caller-owned variable output buffer is used; query states have two/four fixed channels.
Dynamic tree/effect vectors are caller-scoped objects and allocation failures propagate.
Bad lengths, zero strides, out-of-range boundaries and nonfinite jet/checkpoint queries
are rejected. Provider overflow checks remain visible. No public Baseplane ABI changes.

## Reproduce

```sh
cmake -S experiments/moonshot -B /tmp/bp-moon-effects-build \
  -DCE_MOON_SOURCE_DIR=/path/to/Cellerator/experiments/baseplane_moonshot \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/bp-moon-effects-build --target bp_moon_effects_smoke -j 1
ctest --test-dir /tmp/bp-moon-effects-build -R '^bp_moon_effects_smoke$' --output-on-failure
```

The provider must expose `Cellerator::moonshot_effects`. `smoke.cpp` compares E09 against
an independent direct sequence recurrence for several entry states, E10 against a dense
four-channel expansion, E11 against the known nonlinear response with in/out-radius
queries, and E12 against full direct forward/reverse/suffix scans at every boundary.
Validity reset, source identity, seam order and explicit replay costs are covered.
Actual command/source/binary identities appear in `receipt.json`.

All four cards are implemented experimentally, host compiled and run. No family CUDA
kernel was authored, compiled or launched; hardware mappings remain later work. No
benchmark, trained representation, calibrated response or biological efficacy is claimed.
New experimental TUs are outside the configured ctxpp source globs. Canonical inspection
and the local compilation database provided the bounded fallback; generated indexes and
unrelated Todo projections were preserved.
