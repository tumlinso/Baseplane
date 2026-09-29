# CarryFold: make a conditional history composable


**Hypothesis.** A restricted learned state update can remain closed under composition, so input-dependent resets and retention can travel as operands rather than a branch tree. The hierarchy summarizes update operators as well as values.

**Algebra.** With componentwise `T(s)=a*s+b`, earlier L followed by later R is `(R.a*L.a, R.a*L.b+R.b)`. The identity is `(1,0)`. A reset is `(0,b)`. Composition is associative over real arithmetic, not commutative and not bitwise associative for floats. The algorithm does not cover an arbitrary nonlinear recurrence.

**Supplied.** A sequential oracle, a host emulation of shuffle-tree composition, `scan_tiles`, recursive scanning of the tile totals, and `scan_carry`. Scratch is allocated before timing. Tail lanes contribute identities. There is no artificial reset at a warp boundary. The driver already crosses multiple 32-way levels.

**Finish the minimum experiment.** Add a small on-device coefficient/gate producer that consumes current lower-level floats, rather than passing CPU-prepared coefficients as the whole demonstration. Treat it as a parameterized inference probe, not learned biological segmentation. Include producer time. Add a CUB associative scan with the exact same noncommutative operator as the practical competitor. Reuse CUDA 12.9's CUB; do not upgrade CCCL or create a general scan library.

Demonstrate both soft retention and hard resets. Reuse a resulting region/update summary at one coarser scale. Region membership may change with current data; hardware tile boundaries must not silently create semantic heads. Explicit source/sequence boundaries may reset, but their status as such must be preserved.

**Data.** Empty/single items; lengths 31/32/33, 63/64/65, 1023/1024/1025 and beyond; resets immediately before/on/after tile boundaries; no resets; all resets; near-unit retention; alternating strong/weak retention. Shift the same sequence relative to the launch grid and check output after restoring coordinates.

**Quality.** Compare direct sequential state evolution with the composed result over increasing depth. The model family restriction is itself a trade-off: measure a small held-out dependency probe that an averaging summary cannot answer. No claim of general recurrent expressiveness. If a later learning method requires gradients through hard resets, that is a separately explicit estimation choice, not silently ordinary differentiability.

**Measure.** Coefficient production + all scan/carry launches, scratch bytes and numerical error. Include a CUB baseline before claiming speed. The custom kernel can be interesting even if CUB wins; its exposed summary algebra may be reusable.

**Stop.** Stop at the restricted operator family. Nonlinear operators that break closure do not justify an unrestricted GPU interpreter. Record `evaluated_not_promoted` when stability or expressiveness fails. Prior art: P2 and N7; no novelty claim for affine scans.

**Receipt.** `results/scan.json`, explicit numerical tolerance and one two-level dependency probe.
