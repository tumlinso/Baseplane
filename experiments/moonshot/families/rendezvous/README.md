# Real candidate discovery: E17–E20

BP-MOON-060 owns this isolated experimental family. Four concrete host computations
are implemented; the E18 warp lookup also compiles for sm_70. No GPU execution,
benchmark, learned hierarchy or biological relationship is claimed in this receipt.

| Card | Artifact and disposition | Semantic distinction |
| --- | --- | --- |
| E17 | `rendezvous.hpp`: `extract`, `Directory`, `tiled_pairs`; host implemented and run | Every valid sliding window of every supplied sequence enters the directory. No caller-provided candidate packets or keys. Complete groups remain factorized; pair materialization has an explicit capacity. |
| E18 | `PostingHash`, `packed_rows`; host implemented and run. `e18_cuda.cu`, `cuda_smoke.cu`: CUDA compiled, device run pending root assignment | Immutable bucket rows and retained overflow chains replace sorted lookup. Full string equality protects host collisions; GPU keys contain exact two-bit payload and length, limited to 1–32 bases. |
| E19 | `nominate`; host implemented and run | Prefix/suffix sequence sketches nominate different pairs. Their union is deduplicated before full-string Hamming verification. Unnominated relations remain possible. |
| E20 | `subscribe`; host implemented and run | Shared motif cohorts evaluate only requested windows, then route answers with each requester's source, contig, interval and strand restriction. Negative inspected windows remain in support. |

## Source and information contract

`Sequence` uses the installed `PackedFixture` and `SourceMap`; exact canonical
payloads are interpreted only at valid positions. Extraction visits each valid
width-sized sliding window, including cross-word windows. It carries source ID,
contig, source version, local offset, oriented endpoint coordinates and strand.
Reverse strand labels the orientation of the supplied sequence; this family does
not silently reverse-complement it. Exact payloads remain available for revisits.
Directory IDs index extraction results; this is not a universal carrier ABI.

Equal k-mers are exact sequence matches for the retained window, not inferred
regulatory edges. E19's half-window sketches lose information, while subsequent
verification is exact for the declared Hamming-distance predicate. A small
all-pairs recall evaluator is deliberately downstream of nomination and never
supplies candidate keys. The fixture with tolerance two records two nomination
misses even with both probes.

E20 answers preserve version, contig, source, strand and exact endpoint coordinates.
Support lists every inspected window per destination, including negative or
invalid windows. Equivalent predicates share evaluations, but region restrictions
are applied separately. Requests contain motifs and routing destinations; there
is no training-label input. An empty or too-short region inspects no complete
window and therefore has no window evidence.

## Capacity, publication and costs

E17 keeps all postings, checks 64-bit pair arithmetic, and counts complete directed
pair output before materializing at most caller-specified capacity. Tiling is an
executor detail; it imposes no biological boundary. Exhausted capacity skips
further quadratic expansion, with all missing output counted. E18 retains every
collision overflow row. Its immutable GPU table is constructed and copied before
lookup; the supplied harness uses synchronous copies and the default stream.
The library accepts an explicit caller stream and returns launch status; completion
is caller-owned. One complete warp handles each query, including every ballot and
shuffle. Missing keys use an explicit descriptor sentinel, so all-zero DNA keys
remain valid. Device row indices and counts are checked before packing/launch.

E19 is deliberately limited to 4096 objects and two probes. Its candidate sets and
all-pairs recall audit may still take quadratic time and memory within that bound;
large-scale bounded nomination remains later work. E20 bounds emitted answers,
reports produced/stored/dropped and retains complete inspection support. Support
storage itself scales with inspected windows times destinations and is not a
fixed-size GPU buffer.

The smoke reports base inspections, objects, group and directed pair counts,
collision/lookup probes, overflow rows, sketch candidates, verified pairs, recall
comparisons/misses, predicate evaluations, inspected supports and truncated output.
E17 sorting costs O(M log M), with O(M) posting storage and copied exact keys.
E18 builds from that directory; its construction cost therefore includes extraction
and sort, plus collision traversal. Each packed GPU row occupies `sizeof(gpu::Row)`
(32 key/width/group slots plus next-row index), and the upload moves all retained
rows and queries. E19 builds both maps and retains all nominated pairs. E20 scans
source windows per distinct requested motif, comparing only windows in the request
union. These are semantic operation counts, not throughput measurements. Allocation,
packing, upload and final download are visible in the CUDA harness; none was timed.

## Reproduce

From this task's worktree:

```sh
cmake -S experiments/moonshot -B /tmp/bp-moon-rendezvous \
  -DCE_MOON_SOURCE_DIR=/home/tumlinson/Cellerator/experiments/baseplane_moonshot \
  -DBP_MOON_ENABLE_CUDA=ON \
  -DCMAKE_CUDA_COMPILER=/opt/nvidia/hpc_sdk/Linux_x86_64/26.1/cuda/12.9/bin/nvcc \
  -DCMAKE_CUDA_HOST_COMPILER=/usr/bin/g++
python /tmp/moonshot_build_slot.py cmake --build /tmp/bp-moon-rendezvous \
  --target bp_moon_rendezvous_smoke bp_moon_rendezvous_cuda -j1
python /tmp/moonshot_build_slot.py cmake --build /tmp/bp-moon-rendezvous \
  --target bp_moon_rendezvous_cuda_smoke -j1
ctest --test-dir /tmp/bp-moon-rendezvous -R '^bp_moon_rendezvous_smoke$' --output-on-failure
/tmp/bp-moon-rendezvous/families/rendezvous/bp_moon_rendezvous_smoke
```

The CUDA executable is intentionally excluded from CTest. Root may invoke
`bp_moon_rendezvous_cuda_smoke --run DEVICE` after assigning a GPU. It constructs
sequence-derived keys over 400 input bases, forces a single hash bucket with
32-slot overflow rows, and compares GPU lookup for every group against the complete
CPU directory, including a width-distinct absent key. This worker did not invoke it.

Compilation used GCC 13.3.0 and CUDA 12.9.86. The first host compile exposed the
provider-owned `checked_add` namespace; using `ce_moon::checked_add` repaired it
without a shared-source change. Make required a second invocation after CMake
regenerated the newly added CUDA executable target. CUDA emitted its expected
pre-7.5 offline compilation deprecation warning. Both issues are resolved.

`results/host-smoke.txt` records the exact run counters. One CTest passed. Four
host card dispositions are experimental implementations; E18 additionally has a
compiled CUDA library and executable. Device correctness, sanitizer coverage,
wide-scale resource qualification and performance are later obligations.
