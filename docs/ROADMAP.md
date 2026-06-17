# Baseplane Roadmap

Baseplane should grow by adding small, typed biological bit-compute primitives.
It should not grow by absorbing file formats, workflow policy, model logic, or
storage ownership.

## Sequence

Current stable center:

- `dna2` packed words, planes, conversions, mismatch masks, and reverse
  complements.
- Exact motif count and compact emit through `dna2_packed64_view` and
  `motif32_exact`.

Next sequence slices:

- Add `dna2_planes32_stream_view` operations for base masks, GC masks, CpG
  masks, and predicate scans over resident plane streams.
- Add `motif32_allowed` for degenerate motifs using allowed-base bit masks.
- Add multi-motif exact and allowed-motif count APIs.
- Add compact emit variants for multi-motif and degenerate motif scans.

## BitBio

Start with generic packed biological evidence sets:

- packed bitset views
- AND, OR, XOR, AND-NOT counts
- support, specificity, and Jaccard

Higher-level projects can interpret those operations as coaccessibility,
motif-region overlap, clone/state overlap, or perturbation support.

## Segment

Add offset/index ragged views and reductions:

- segment popcount
- segment sum
- segment max
- segment emit-if

Baseplane owns the ragged primitive, not the biological interpretation of the
segments.

## Emit And Runtime

Sparse outputs should use compact emission with explicit capacity and overflow
accounting. Runtime helpers may be added when they make residency explicit:
device id, stream, allocation size, and synchronization must remain visible.
