# Evidence before promotion

## Three separate questions

1. Does the implementation compute the defined operator correctly?
2. Does its complete execution strategy improve a cost at the same semantics/quality?
3. Does that operator preserve information useful to a sequence-modeling question?

Answering the first does not answer the other two. Existing exact-count throughput is not an embedding baseline.

## Execution and timing

Use one scheduler-assigned V100, CUDA12.9, sm_70, fixed compiler/options and recorded source hashes. Build outputs are task-specific; GPU benchmark/profiler/sanitizer sessions are serialized under the existing authority. Device correctness may use its assigned device; no hard-coded physical index. Check quiescence before and after. Do not log another agent's run as your evidence.

The supplied event timer covers a complete **device-resident kernel pipeline** with buffers prepared. Rethread includes resets, cheap outputs, queue production and consuming work. CarryFold includes every scan/carry launch. Allocation, initial host construction, H2D/D2H and final result interpretation are outside that interval; report them separately for end-to-end conclusions. The scalar-oracle run is also outside timed GPU work. A single kernel time is a diagnostic, not the pipeline verdict.

Warm up; record individual samples, median, p95 and absolute milliseconds. Randomize/interleave comparison order in the final measurement; the supplied driver executes variants in a fixed order and is only an initial harness. Record n's meaning: BitLift n is 32-item predicate tiles; other cases use current representation records. Never relabel record rate as bases/second without an exact conversion and the cost of producing those records.

## Correctness

Scalar oracle first, then device comparison, then memcheck/initcheck/racecheck/synccheck using the matching supported sanitizer. Test empty, singleton, invalid/tail masks, 31/32/33 and coarser boundaries. Every named warp participant must execute the right collective. A shuffle cannot read a nonparticipating lane. Shuffles/votes are not a general memory fence. Preserve producer/consumer ordering, scratch ownership, capacity/error counters and original identities. Invalid NaNs are not masked safely merely by multiplication by zero; define validity first.

Float sums/scans have a stated error envelope, not bitwise associativity. Use no fast-math default; precision relaxation is an explicit comparison. Avoid artificial zero work as a favorable baseline. Compare branch, predicated and compacted formulations fairly where relevant; inspect real instructions, register pressure/spills and memory traffic. A compiler may already emit LOP3 or warp-aggregated atomics.

## Representation tests

Use synthetic data to isolate failures, not to claim biology. Include sequence perturbations that retain local counts but change ordering; rare relevant combinations that a cheap gate might miss; state-conditioned changes; remote partners with nearby distractors; random placement shifts and alternate tilings. Keep held-out probe seeds separate. Label any oracle segmentation/candidate list as an upper-bound fixture.

All source input can remain accessible without being encoded losslessly in a small vector. Report probe quality at matched computation/representation budget, any exact-source revisits and their cost, selection/grouping errors, invalid/rare-case behavior and total intermediate memory. For whole-genome extrapolation include the first linear scan and lower-level storage; do not claim a sublinear first embedding pass.

## Result contract

Use results/TEMPLATE.json as shape, create one results/<case>.json only after running the experiment. It needs correctness, CUDA, sanitizer, baseline, costs, quality, limitations and verdict fields, with paths to real receipts. `evaluated_not_promoted` does not require a speedup. Absence of a GPU is `blocked`, never a passed test. The checker can validate evidence presence but cannot replace scientific inspection.

Keep scope small: expand the provided matrix only to explain a failure or crossover. Do not insist on four successful mechanisms, every GPU, production integration or a complete training run.

After actual review, copy `SELECTION_TEMPLATE.json` to `selection.json`. Choose at most two candidates; an empty selection with a supported negative report is valid. Selected mechanisms need a bounded executed two-level probe. The result checker accepts `status: completed` and `verdict: promote_candidate` or `evaluated_not_promoted`, with nonempty local paths to supporting evidence. It checks presence and record consistency, not the honesty or scientific validity of a claim.

Save supporting evidence for each mechanism under `results/<case>/`, with the main record at `results/<case>.json`. Gate fingerprints deliberately exclude generated results from source-compilation gates. After shared-kernel/driver changes, rerun the affected correctness and measurement gates against the final source hashes before comparing results; never promote a stale receipt.
