# BitLift: use Boolean evidence without unpacking it into expensive work


**Hypothesis.** A register/bit-parallel predicate basis can supply useful numerical features more cheaply than expanding every child into a separate floating operation. This is a representation/execution experiment, not a claim that histograms encode all sequence information.

**Supplied.** `Planes` holds three 32-bit predicate planes plus a validity mask. `bitlift_warp` maps eight patterns × four channels over a warp; fixed `lop3` and popcount precede a floating weighted sum. `bitlift_thread` is the ordinary-operator GPU comparator. Both consume the same weights and implement the same function. Scalar bit-by-bit and bitset references are independent.

**Finish the minimum experiment.** Compile and sanitize both paths. Add a tiny adapter that gets the three predicate planes from (a) the existing packed/valid sequence representation and (b) threshold tests on 32 lower-level floating records. Predicates and thresholds are parameters, not named motifs or fixed genomic annotations. Retain the exact lower inputs and source/child mappings. Feed the second result through the same operator once; no generic graph library.

Use no more than three plane/basis configurations. Compare one-thread-per-tile and one-warp-per-tile before trying inline PTX variations; the C++ compiler may already generate LOP3. Inspect actual SASS to verify the intended instructions and check register usage, not the source spelling.

**Baseline.** Identical bit basis, validity semantics, weights, output dtype and normalization. Include unpack/plane construction on both sides where required. Do not compare prepared bit planes to an unprepared dense input without separating preparation costs. Add an ordinary small dense projection only as a clearly different representation/quality comparator.

**Data.** All eight patterns at lengths 0–32; arbitrary holes in validity; low/high bit density; repeated and mixed windows; shifted windows spanning packed-word boundaries. For hierarchy, supply current float children with same signs but different magnitudes to expose what thresholding loses. Synthetic order-sensitive labels distinguish arrangements that have the same histogram.

**Failure witness already supplied.** Rotating the predicate positions changes the input but leaves the histogram embedding unchanged. This is intentional and should stay in tests. Source revisit can repair the missing detail only at an explicitly counted cost; provenance alone is not an information-retention metric.

**Measure.** Preparation, kernel and full resident pipeline time; temporary bytes; registers/spills; instructions per tile; representation error on the order-sensitive probe. Hardware sizes are performance dimensions, not biological segments. Parameter gradients of the weighted sum are elementary, but no training/autograd framework belongs in the task.

**Stop.** Promote only as a bounded side-channel/primitive if the cost-quality comparison justifies it. Keep `evaluated_not_promoted` if ordinary bit logic wins, thresholds lose too much, or the warp mapping wastes more lanes than it saves. Do not expand to a Boolean virtual machine, a motif database or genome-scale pre-discovery.

**Receipt.** `results/bitlift.json` plus small supporting logs. Reference sources: N1–N5; the proposed composition is this lab's synthesis.
