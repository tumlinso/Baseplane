# Adaptive and refinable hierarchy probes

`hierarchy.hpp` and `smoke.cpp` implement four sequence-grounded CPU experiments. The isolated parent build discovers this family automatically. These are bounded synthetic mechanisms, with no performance or biological claim.

- **E13, implemented experimentally:** exact valid GC/AT carriers feed Cellerator's `lift`/`unlift`. Coarse values and residuals occupy separate vectors. The coarse-only expansion never reads residual values. A retained reservoir roundtrips; omission changes the threshold answer; replay restores it from an explicitly versioned source. This first probe uses one lifting level, with a scalar unpaired tail. Full residual retention is additional storage, not compression.
- **E14, implemented experimentally with Cellerator learning:** caller-owned sequence features feed the Cellerator logistic scorer, with regularization and a positive-rate compute-budget penalty. The bounded synthetic training target marks GC regime transitions. Inference sees four local features and fitted weights, with explicit minimum and maximum chunk lengths; it has no training labels or biological annotations. The probe compares fixed windows, surprise cuts and learned cuts after insertion, then reconstructs exact bases through the chunk map. Scorer fitting is owned by Cellerator. A build without its header explicitly reports E14 unavailable.
- **E15, implemented experimentally:** an integer GC count in a span of at most 1024 bases supplies eleven successive precision planes, scaled by an explicit denominator of 1024. A strict threshold decision fetches more planes only when its conservative interval is ambiguous. This certifies this count query; it does not establish that coarse bits preserve arbitrary learned latent tasks. The probe emulates progressive fetch on CPU; it has no device bitplane storage backend.
- **E16, implemented experimentally:** four-character rolling fingerprints select seams with hard length caps. A second offset forest supplies alternative seams. Exact motif queries use the alternative or replay crossing windows, and insertion reuse is checked by exact chunk bytes. Hashes select boundaries and never establish content identity. The final short tail may fall below the minimum; all chunks satisfy the maximum. Repetitive sequence exercises forced cuts.

`SourceMap` retains source ID, contig, origin, version and strand. All spans are half-open local input spans; `support` returns only valid source coordinates. Invalid tokens remain recoverable through E14's exact inverse map but contribute no GC carrier or motif evidence. Tail payloads never become biological evidence. Reverse-strand coordinates are tested. Source replay rejects a different source descriptor/version; callers must increment source version when content changes.

Caller-owned exact emission tests distinguish produced, stored and dropped counts. CPU vectors allocate in proportion to the bounded fixture and are research containers, without a production capacity ABI. The receipt reports residual bytes, precision fetch count, chunk reuse and seam replay work; it is not a timing measurement. Motif replay counts candidate windows even if the eventual exact comparison fails. Query replay here is intentionally exhaustive and establishes semantics, not efficient candidate routing. Retaining source lineage alone makes no lossless-embedding claim.

Build from the repository root, supplying explicit provider paths:

```sh
cmake -S experiments/moonshot -B /tmp/bp-moon-hierarchy-build \
  -DCE_MOON_SOURCE_DIR=/path/to/Cellerator/experiments/baseplane_moonshot \
  -DCE_MOON_LEARNING_DIR=/path/to/Cellerator/experiments/baseplane_moonshot/families/learning \
  -DBUILD_TESTING=ON
cmake --build /tmp/bp-moon-hierarchy-build --target bp_moon_hierarchy_smoke -j1
ctest --test-dir /tmp/bp-moon-hierarchy-build -R '^bp_moon_hierarchy_smoke$' --output-on-failure
```

See `receipt.md` and `semantic-output.txt` for actual commands, source identities and observed output. CUDA compilation, GPU execution, benchmark timing, multi-level learned hierarchy, physical compressed precision storage and trained biological evaluation remain later work.
