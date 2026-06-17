# Baseplane Agent Guide

## Project Identity

Baseplane is a low-level biological bit-compute library.

Its job is to map biological primitives onto hardware primitives before those data become matrices, graphs, models, or workflows.

The core thesis is:

> Baseplane maps biological structure onto GPU/CPU bit operations.

Baseplane is not a genomics framework. It is not a file-format library. It is not Cellerator. It is not CellShard. It should remain small, sharp, inspectable, benchmarkable, and brutally honest about memory traffic.

A good Baseplane primitive should make a sentence like this true:

- A DNA window is a 2-bit word.
- A motif match is XOR / OR / POPCOUNT.
- A degenerate motif is an allowed-base bitplane.
- A CpG context is a shifted mask intersection.
- A chromatin evidence set is a packed bitset.
- A cis-window is a segment.
- A candidate regulatory event is a compact emitted record.
- A repeated biological scan is a resident GPU workflow.

If a change does not strengthen this identity, question it.

## Current Center of Gravity

The current foundation is `dna2`: compact DNA/RNA sequence primitives using canonical 2-bit encoding.

Encoding convention:

```text
A = 00
C = 01
G = 10
T/U = 11
```

Current representations:

```text
dna2_word32       16 bases packed into uint32_t
dna2_word64       32 bases packed into uint64_t
dna2_planes32     32 bases split into lo/hi uint32_t bitplanes
dna2_planes64     64 bases split into lo/hi uint64_t bitplanes
dna2_inlplane32   16-base inline serialized plane word
dna2_inlplane64   32-base inline serialized plane word
```

Preserve the central distinction:

```text
Packed words are how sequence lives in memory.
Bitplanes are how sequence becomes a warp-level biological decision.
```

But do not force bitplanes everywhere. For exact shifted-window scanning, packed `uint64_t` windows may be the fastest representation. For biological predicate logic, degenerate motifs, CpG/GC masks, and grammar decisions, bitplanes are often the right compute representation.

## Scope

Baseplane owns low-level primitives for:

- 2-bit DNA/RNA packing and unpacking.
- Packed sequence windows.
- Bit-plane transforms and masks.
- Reverse complements.
- Exact and approximate mismatch counting.
- Motif-window operations.
- Degenerate allowed-base motif primitives.
- Sequence predicate masks such as base masks, GC masks, and CpG-start masks.
- Packed biological bitsets.
- Bitset evidence operations: AND, OR, XOR, AND-NOT, POPCOUNT, Jaccard, support, specificity.
- Generic segmented reductions over biological ragged structures.
- Compact event/edge emission primitives.
- CPU scalar, optional CPU SIMD, and optional CUDA kernels.
- Thin residency/runtime helpers when they expose memory placement and avoid hidden costs.

Baseplane does not own:

- FASTA/FASTQ/BAM/VCF/BED/GTF/GFF parsing.
- Genome annotation semantics.
- Motif databases.
- PWM model policy.
- Biological ontology policy.
- AnnData, HDF5, Zarr, CSPACK, CellShard, or Cellerator storage behavior.
- Cellerator model workflows.
- Biological interpretation of outputs.
- Training loops, autograd, or model policy.

Higher-level projects may use Baseplane to implement biological meaning. Baseplane should provide the fast, typed, low-level operations those projects compose.

## Repository Direction

Grow Baseplane around four primitive families:

```text
seq       Compact DNA/RNA sequence words, planes, motifs, reverse complements, scans.
bitbio    Packed biological evidence sets and set-count operations.
segment   Ragged biological grouping primitives: offsets, indices, reductions.
emit      Compact event and edge emission without dense intermediate buffers.
runtime   Minimal residency helpers for CUDA streams, device views, and repeated scans.
```

Recommended future layout:

```text
include/Baseplane/
  core/
    config.hh
    status.hh
    bit_ops.hh
    span.hh
  seq/
    dna2_types.hh
    dna2_views.hh
    dna2_motif.hh
    dna2_ops.hh
    dna2_scan.cuh
    dna2_plane_stream.cuh
  bitbio/
    packed_set.hh
    set_ops.cuh
    support.cuh
  segment/
    segment_view.hh
    segment_reduce.cuh
  emit/
    compact_emit.cuh
    edge_emit.cuh
  runtime/
    cuda_context.cuh
    resident_sequence.cuh

src/
  seq/
  bitbio/
  segment/
  emit/
  runtime/

bench/
  seq/
  bitbio/
  segment/
  emit/

tests/
  seq/
  bitbio/
  segment/
  emit/

docs/
  SEQUENCE_BITS.md
  BITBIO.md
  PERFORMANCE_CONTRACT.md
  BASEPLANE_PRIMITIVES.md
  ROADMAP.md
```

Do not reorganize everything at once unless explicitly asked. Prefer narrow changes that move the repo toward this structure without breaking existing targets.

## Design Laws

### 1. Biological unit first

Every kernel or primitive should document the biological unit represented by its parallelism.

For major CUDA kernels, describe:

```text
lane semantics
warp semantics
block semantics
grid semantics
input layout
output layout
memory traffic expectation
```

Examples:

```text
thread = shifted sequence window
warp   = independent window batch
block  = reduction tile
grid   = full sequence scan
```

or:

```text
lane = base position / predicate / local candidate
warp = 32-base regulatory word / biological decision
block = local biological context
grid = genome-scale scan
```

### 2. Representation must match the operation

Use packed `dna2_word64` shifted windows for fixed-width exact or Hamming-style motif scans.

Use split planes for biological predicate logic:

- base masks
- GC/CpG masks
- degenerate motifs
- local grammar
- base-class operations
- rule masks

Use packed bitsets for biological support/count evidence:

- accessibility support
- coaccessibility
- motif-region overlap
- cell-group overlap
- clone/state/perturbation overlap

Use segmented structures for ragged biology:

- gene to cis-regulatory elements
- TF to target regions
- pathway to genes
- clone to cells
- neighborhood to cells

Use compact emit for sparse biological outputs:

- motif hits
- candidate edges
- filtered records
- top biological events

### 3. No hidden dense outputs

Dense byte-per-window outputs are opt-in only.

Default scan modes should be:

```text
count-only
compact-hit-list
bitmask/tile-summary when justified
```

Do not write one dense output element per window unless the user explicitly requested that output mode or a test requires it.

### 4. No hidden allocation in hot paths

Hot kernels and hot wrapper APIs must not secretly allocate/free device memory on every call.

Accept explicit pointers, spans, views, buffers, stream handles, and output storage.

If a convenience wrapper allocates, name it as a convenience wrapper and keep it outside the core hot path.

### 5. Measure memory traffic separately from kernel work

Benchmarks must distinguish:

```text
host preparation time
H2D upload time
kernel time
D2H readback time
end-to-end time
```

Do not claim primitive throughput from a benchmark dominated by upload, download, allocation, synchronization, or dense output materialization.

### 6. CUDA is preferred but optional

CUDA is the preferred hot path, but Baseplane must build without a CUDA compiler.

CPU scalar reference paths are mandatory for correctness.

CPU SIMD backends are optional and must not leak backend-specific types into public headers.

### 7. V100 / sm_70 remains the native tuning baseline

The current native tuning target is Volta `sm_70`, especially Tesla V100.

Do not make the primary implementation depend on Ampere/Hopper-only features.

Future architecture-specific optimizations are welcome behind explicit compile-time capability checks or separate backends.

Examples:

```text
sm_70 baseline: packed loads, bitwise ops, popcount, shared-memory reductions, grid-stride loops.
sm_80+ optional: cp.async-style pipelines when useful.
sm_90+ optional: DPX-style dynamic programming when useful.
```

### 8. Public API before benchmark-only kernel growth

If a benchmark kernel proves useful, wrap it in a public or semi-public API before adding more benchmark-only variants.

A benchmark should exercise the same logical API that downstream users will call, unless the purpose is explicitly exploratory.

### 9. Keep Baseplane low-level

It is okay for a function to be biologically inspired. It is not okay for Baseplane to own high-level biological policy.

Good Baseplane names:

```text
scan_exact_count
scan_allowed_count
bitset_and_count
segment_popcount
compact_emit
```

Names to avoid in Baseplane unless explicitly directed:

```text
annotate_enhancers
call_peaks
score_regulons
build_cell_trajectory
infer_gene_program
```

Those belong in Cellerator or higher-level tools.

## Preferred Public API Style

Use explicit view types and pointer-plus-size structures.

Good:

```cpp
struct dna2_packed64_view {
    const std::uint64_t* words;
    std::uint64_t n_bases;
    std::uint64_t n_words;
};

struct motif32_exact {
    std::uint64_t packed;
    std::uint64_t active_fields;
    std::uint8_t length;
    std::uint8_t max_mismatches;
    std::uint16_t motif_id;
};
```

Good CUDA-facing shape:

```cpp
baseplane_status scan_exact_count_cuda(
    cudaStream_t stream,
    dna2_packed64_view sequence,
    motif32_exact motif,
    unsigned long long* device_count);
```

Avoid APIs that hide residency or allocation:

```cpp
// Avoid for hot paths.
auto hits = scan(sequence_string, motif_string);
```

Convenience APIs are allowed only as wrappers around explicit hot-path APIs.

## Sequence Primitive Roadmap

### Immediate priority: stabilize exact scan

The current shifted packed scanner should become a stable library primitive.

Implement or maintain:

```text
dna2_packed64_view
motif32_exact
scan_exact_count_cuda
scan_exact_count_cpu
scan_exact_count_highway, if SIMD is enabled
```

Benchmarks should call these wrappers instead of calling raw kernels directly.

### Next: plane stream

Add a first-class plane-stream representation:

```text
dna2_planes32_stream_view
lo_words[]
hi_words[]
```

Use it for:

```text
base masks
GC masks
CpG masks
degenerate motifs
sequence predicate scans
```

### Next: degenerate motif primitive

Represent allowed-base motifs as four bit masks:

```cpp
struct motif32_allowed {
    std::uint32_t allow_A;
    std::uint32_t allow_C;
    std::uint32_t allow_G;
    std::uint32_t allow_T;
    std::uint32_t active_mask;
    std::uint8_t length;
    std::uint8_t max_mismatches;
    std::uint16_t motif_id;
};
```

Matching should be bitwise set membership over plane-derived base masks, not expansion into many exact motifs unless a benchmark proves expansion is better for a specific case.

### Next: multi-motif scanning

Real biological use often scans batches of motifs.

Add:

```text
motif32_exact_batch_view
motif32_allowed_batch_view
scan_exact_multi_count_cuda
scan_allowed_multi_count_cuda
scan_*_emit_cuda
```

Avoid one full sequence pass per motif when motif batches are large enough to benefit from reuse.

## BitBio Roadmap

Baseplane should own packed biological bitset operations.

Start with:

```text
packed_bitset_view32
packed_bitset_view64
bitset_and_count
bitset_or_count
bitset_xor_count
bitset_andnot_count
bitset_jaccard
bitset_support
bitset_specificity
```

These primitives should be generic. Cellerator can interpret them as coaccessibility, motif support, group enrichment, clone overlap, perturbation overlap, or state overlap.

Baseplane should not know those biological labels.

## Segment Roadmap

Add offset/index-based ragged views:

```cpp
struct segment_view {
    const std::uint32_t* offsets;
    const std::uint32_t* indices;
    std::uint32_t segment_count;
    std::uint32_t index_count;
};
```

Implement:

```text
segment_popcount
segment_sum
segment_max
segment_emit_if
```

Use this to support higher-level biological structures without owning their semantics.

## Emit Roadmap

Sparse biological outputs should be compact by default.

Implement generic compact emission:

```text
predicate -> prefix/rank -> compact record
```

Useful output records may include:

```cpp
struct motif_hit {
    std::uint32_t position;
    std::uint16_t motif_id;
    std::uint8_t mismatches;
    std::uint8_t strand;
};
```

Keep overflow behavior explicit:

```text
capacity reached
records written
records dropped
required capacity estimate, if available
```

Do not silently truncate without status.

## Runtime Roadmap

Add minimal runtime helpers only when they preserve transparency.

Useful runtime objects:

```text
cuda_context
resident_sequence
resident_motif_batch
resident_bitset
scratch_buffer
```

They should make this possible:

```text
upload once
scan many times
read back compact results or small counts
```

They should not hide:

```text
device id
stream
allocation size
residency
host/device transfer
synchronization
```

CUDA Graph capture may be added later for repeated scan workflows, but only after the underlying API is stable.

## CUDA Kernel Policy

For CUDA kernels:

- Prefer grid-stride loops for large scans.
- Prefer packed device-resident words for exact shifted-window scans.
- Prefer device-side reductions for count-only output.
- Prefer sparse compact output over dense hit buffers.
- Avoid per-window scalar reconstruction when shifted word construction is possible.
- Avoid per-call `cudaMalloc` / `cudaFree` in benchmarked paths.
- Use explicit dynamic shared memory only when it pays for itself.
- Record register count, shared memory use, and kernel timing when changing hot kernels.
- Keep kernels in narrow translation units so they are easy to profile.

For reductions:

- Use block-local reduction before global atomics.
- Consider two-stage reductions for multi-motif counts or high-contention outputs.
- Do not optimize atomics blindly; measure contention and output density.

For warp-level primitives:

- Use warp ballots when lanes correspond to bases, predicates, or local candidates.
- Use shuffles for local sequence grammar or lane-neighbor exchange.
- Use match/group primitives for equivalence patterns only when there is a clear use case.
- Do not use warp-level collectives just because they are elegant. Use them when they match the biological operation and the performance profile.

## CPU / SIMD Policy

The scalar path is the reference implementation.

Optional SIMD paths should:

- Keep backend-specific types inside backend translation units.
- Preserve the same semantics as scalar and CUDA paths.
- Avoid changing public data structures for one SIMD backend.
- Be benchmarked separately from scalar.

Highway is allowed as an optional backend. Future AVX2, AVX-512, NEON, or other CPU paths should follow the same isolation rule.

## CMake and Packaging Policy

Keep public configuration simple:

```bash
cmake -S . -B build -DBASEPLANE_ENABLE_CUDA=ON
cmake --build build -j
```

Local machine-specific compiler paths, host compiler choices, and profiling presets should move toward `CMakePresets.json` or developer documentation.

Avoid hardcoding workstation-specific paths in ways that surprise public users.

The installable CMake target should remain:

```cmake
Baseplane::seq
```

If new module targets are added later, keep names narrow and predictable:

```cmake
Baseplane::bitbio
Baseplane::segment
Baseplane::emit
Baseplane::runtime
```

Do not create target sprawl prematurely.

## Testing Requirements

Every primitive must have a CPU reference test.

CUDA tests must compare against CPU references for:

- random inputs
- boundary lengths
- zero-length or empty cases, where applicable
- maximum supported window sizes
- reverse-complement behavior
- dense-hit vs count-only consistency, where applicable
- compact-output correctness, where applicable

For sequence primitives, include tests for:

```text
all A/C/G/T cases
random windows
length masks
mismatch masks
reverse complements
shifted windows crossing word boundaries
```

For bitsets, include tests for:

```text
all-zero
all-one
single-bit
unaligned tail bits
random masks
large masks
```

For segment reductions, include tests for:

```text
empty segments
single-element segments
large segments
highly skewed segment sizes
random ragged layouts
```

## Benchmark Requirements

Benchmark and profiler runs must be reproducible.

Record:

```text
hardware
CUDA version / compiler
build flags
command line
input size
motif length / count / degeneracy
hit density
kernel time
end-to-end time
H2D bytes
D2H bytes
registers per thread
shared memory
number of devices
```

Use the benchmark mutex when running benchmark-producing jobs on shared systems.

Do not overwrite benchmark claims without recording the commands and context that produced the new numbers.

Benchmark categories to maintain over time:

```text
single exact motif count
single exact motif compact emit
multi exact motif count
degenerate motif count
plane predicate scan
reverse-complement scan
bitset support count
bitset Jaccard/coactivity
segment reduction
compact event emission
resident repeated-scan workflow
```

## Documentation Requirements

When adding a primitive, update or create the relevant doc.

Minimum docs:

```text
docs/SEQUENCE_BITS.md
docs/BITBIO.md
docs/PERFORMANCE_CONTRACT.md
docs/BASEPLANE_PRIMITIVES.md
docs/ROADMAP.md
```

Each primitive doc should answer:

```text
What biological/computational object does this represent?
What memory layout does it require?
What GPU/CPU primitive does it map to?
What output modes exist?
What are the non-goals?
What benchmark validates it?
```

## Relationship to CellShard and Cellerator

Baseplane should sit underneath Cellerator and beside CellShard.

```text
Baseplane   = primitive biological bit-compute kernels.
CellShard   = storage, sharding, pack generation, runtime delivery.
Cellerator  = biological interpretation, models, workflows, and GPU execution composition.
```

Baseplane may expose sections or views that CellShard can store later, but Baseplane should not own CellShard formats.

Baseplane may provide primitives that Cellerator interprets as chromatin annotation, regulatory evidence, enhancer-gene linking, or trajectory gating, but Baseplane should not own those high-level biological meanings.

## Good Changes

Good changes usually look like:

- A new compact type with clear layout.
- A CPU reference implementation.
- A CUDA implementation that avoids hidden memory traffic.
- A benchmark that separates kernel time from transfer time.
- A doc explaining the biological-compute mapping.
- A test proving semantic equivalence across scalar/CUDA/SIMD paths.
- A public wrapper around a useful kernel.

## Bad Changes

Bad changes usually look like:

- Adding a parser before a primitive.
- Adding biological policy before a low-level representation.
- Adding a dense output by default.
- Adding hidden allocation in a hot path.
- Adding a benchmark-only kernel without a route to public API.
- Adding a new backend that leaks backend types into public headers.
- Adding a clever CUDA trick without a biological mapping or benchmark.
- Optimizing for future hardware while breaking the V100 baseline.
- Turning Baseplane into a Cellerator workflow module.

## Immediate Next Tasks

The next useful work should be narrow:

1. Add explicit `dna2_packed64_view` and `motif32_exact` types.
2. Add a public `scan_exact_count_cuda` wrapper around the shifted packed scanner.
3. Update the CUDA benchmark to call the public wrapper.
4. Add `docs/PERFORMANCE_CONTRACT.md`.
5. Add `docs/ROADMAP.md` with the primitive roadmap.
6. Then add `dna2_planes32_stream_view`.
7. Then add `motif32_allowed` and degenerate motif count scanning.

Do not begin with file formats, parsers, large workflows, or broad API redesign.

## Decision Rule for Agents

When unsure, ask this:

> Is this a low-level biological primitive whose memory layout directly enables a fast CPU/GPU operation?

If yes, it may belong in Baseplane.

If it requires genome annotation semantics, dataset storage policy, model interpretation, biological workflow logic, or user-facing analysis policy, it probably belongs above Baseplane.

Keep Baseplane small enough that every primitive can be understood, tested, profiled, and trusted.
