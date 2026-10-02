# Reuse under context and sequence changes

`incremental.hpp` implements four bounded CPU experiments; `smoke.cpp` exercises their semantic distinctions. All cards E33–E36 are implemented experimentally. This is an isolated research family, with no production API, biological validation or speed claim.

**E33: repeat quotient with contextual residuals.** Hash nominations select candidates, followed by exact original-token comparison. The fixture deliberately forces every nomination to collide. Identical content shares one Cellerator DFA effect and a sequence GC count. Occurrences retain independent source descriptors, state versions and sequence-specific scalar context residuals. Identical packed words with different invalidity sidecars remain distinct because `NCGT` and `ACGT` retain different original tokens. Sharing a local effect never authorizes sharing the context answer. This probe does not learn context residuals.

**E34: variant dirty cones.** A source edit changes exact directory membership and marks source/effect/ancestor/query and membership/query nodes through reverse dependencies. The old posting is removed and the new posting is inserted. Source identity and a consecutive source version are checked before mutation. A visited frontier reaches a fixed point, including cyclic dependencies. The probe shows that a DFA value may remain unchanged after an edit while structural memberships still require invalidation, and that an independent sibling remains untouched. This is a scalar dirty-set and posting update, with a scalar effect recomputation; no asynchronous recomputation scheduler was added. Complete dependencies must include affected old and new bucket queries, boundary changes and global negative evidence. The supplied eight-node example has a directory-wide membership node covering both buckets; an incomplete dependency graph would invalidate these guarantees.

**E35: query-support memoization.** A result stores query, weights, cell-state and representation versions, source identity/version, and all inspected dependency spans. It requires a complete ordered edit chain to cross source versions. An edit outside support can preserve the answer; changes to inspected negative evidence, missing/skipped versions and changed query/weights/state/representation invalidate it. Coordinate-changing insertions invalidate affected support; reverse views conservatively reject all length changes because their coordinates use the full source extent. A separate cached plan stores only the span and reruns the count, so plan reuse cannot return a stale result. Edit logs are trusted caller evidence and must describe every change. Within an unchanged source version, callers are responsible for preserving content.

**E36: grammar-compressed effects.** Exact token leaves and ordered child-pair tuples form a shared DAG, with Cellerator composing cached DFA effects. A static balanced parse and incremental phrase-pair interning produce the same exact effect. Repeated phrases share multiple levels while occurrence source spans remain separate. The exact grammar expands every original token, including invalid symbols; the DFA resets on invalid bases and deliberately forgets older context. The grammar is lossless syntax for retained tokens, while its DFA effect is a limited query representation. This is a small pair dictionary rather than a learned biological hierarchy or whole-genome grammar optimizer.

All source descriptors retain source ID, contig, origin, version, strand and length. Counts ignore invalid bases; source spans still include the positions needed to recover invalid tokens. Forward and reverse coordinates and coordinate overflow are tested. These experiments use fixture-sized owning vectors/maps and throw for invalid descriptors, spans, versions and arithmetic overflow. There is no truncating output buffer; memory follows the input/dependency/grammar size. No constant-time edit or compression-ratio claim is made. Receipts expose exact comparison, dirty node, inspected support and grammar composition counters; allocation, grammar construction and exact expansion remain charged work for later measurements.

Build through the shared isolated substrate:

```sh
cmake -S experiments/moonshot -B /tmp/bp-moon-incremental-build \
  -DCE_MOON_SOURCE_DIR=/path/to/Cellerator/experiments/baseplane_moonshot \
  -DBUILD_TESTING=ON
cmake --build /tmp/bp-moon-incremental-build --target bp_moon_incremental_smoke -j1
ctest --test-dir /tmp/bp-moon-incremental-build -R '^bp_moon_incremental_smoke$' --output-on-failure
```

See `receipt.md` for source identities and actual commands and `semantic-output.txt` for observed output. CUDA compilation/execution, parallel dirty queues, persistent caches, biological context fitting, compressed storage, broad timing and full incremental hierarchy rebuilding remain later work.
