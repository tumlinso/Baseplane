# Rethread: let informative objects recruit a whole warp


**Hypothesis.** The right parallel granularity changes with representational depth. Cheap evidence can be lane-parallel across objects; selected richer floating computation can be channel-parallel within an object. Online compaction may bridge those granularities economically.

**Supplied.** A ballot/rank producer builds an unordered bounded queue. A separate same-stream consumer assigns one warp per selected 4→32→32 floating transform. Every record keeps its cheap residual; outputs return to original indices. Queue capacity, attempted count and dropped count are explicit. The three GPU paths are (1) per-thread branch + full transform, (2) uncompacted warp-per-record transform, and (3) compacted warp-per-selected-record transform.

The second baseline matters: a win over a poor per-thread mapping does not establish a win from compaction. Likewise a simple atomic baseline may already be warp-aggregated by the compiler. [N5]

**Finish the minimum experiment.** Compile and sanitize all three equivalent paths. Add the bundled-CUB selection path with the same unordered-output allowance or document the stability difference. Reuse the same selection/refinement at a coarser level of floating records: lane identity changes, but source identity must survive. Keep one shared micro-transform and two levels, not a configurable mixture-of-experts framework.

Gate from current values. Probe a range of densities without claiming the deterministic threshold is learned or biologically calibrated. For a minimal quality test, use a held-out synthetic task where rare nonlocal combinations matter; report cheap-path errors and the count/cost of revisits. More uncertain or high-entropy input is not automatically more biologically important.

**Data.** Selection densities near 0%, 1%, 5%, 25%, 50%, 100%; contiguous vs scattered selection; empty/single/tail inputs; all records of one shape vs two small shapes only if warranted. Test queue capacity zero/too small separately and never treat overflow as a successful inference. Verify every original output, including nonselected records.

**Execution.** Separate producer/consumer launches establish ordering. No published queue consumer runs concurrently with an incomplete producer. No device-side allocation, spin termination detector, dynamic parallelism or cooperative-grid requirement. Device index and stream belong to the caller/driver; scheduling uses the existing GPU owner. Basic bounded grid-stride processing is not a new persistent-runtime project.

**Measure.** The resident pipeline includes counter resets, cheap-path writes, queue production, launch overhead and all selected work. Report allocations and H2D/D2H separately. Record occupancy, registers, spills, bytes moved and gate density. Do not infer work saving from the absence of jumps in SASS.

**Conditional extension, only after a promising result.** Batch sufficient compatible selected rows into a Volta WMMA tile and compare against the already-correct warp FMA path including padding/packing/precision conversion. No separate task until evidence justifies it; no promised Tensor Core win. [N8, P4]

**Stop.** If compaction costs more than it avoids, prefer the uncompacted warp formulation for that regime and retain the negative threshold result. This is probably the first experiment to run because it most directly tests changing SIMT organization with information depth.

**Receipt.** `results/rethread.json` with density-specific rather than universal verdict.
