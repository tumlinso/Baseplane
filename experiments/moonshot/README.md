# Sequence moonshot research library

The workshop in `BP-MOON-RUN-1`, paired with Cellerator `CE-MOON-RUN-1`, now contains
48 bounded host representatives across 12 families and five connected
compositions. These are experimental sequence representations and execution paths;
the intended learned genome hierarchy remains a research objective. Cellerator
owns numerical and learned providers, Baseplane owns exact sequence grounding.
The separately named `CE-MOON-RUN-V1` and GlassHelix campaigns have their own
unchanged authority.

Start with the [handoff](../../planning/baseplane_moonshot_bootstrap/results/baseplane-handoff.md)
and [48-card inventory](../../planning/baseplane_moonshot_bootstrap/results/catalogue.json).
Every inventory row records actual source, semantic comparison, loss/revisit
behavior, CUDA/GPU coverage and remaining variants. Family receipts retain their
original observations; later root checks are additive.

| Family | Cards | Runnable representative and retained limits |
|---|---|---|
| [Logic](families/logic/README.md) | E01–E04 | LUT circuits, carry counters, strand grammar and sparse query routing; general grammar/learned circuits remain variants. |
| [Automata](families/automata/README.md) | E05–E08 | Finite effects, witnesses, bounded grammar and distinct weighted alternatives; device executors and calibrated weights remain. |
| [Effects](families/effects/README.md) | E09–E12 | Affine/block responses, approximate jets and checkpoints; no certified jet bound or trained continuous hierarchy. |
| [Hierarchy](families/hierarchy/README.md) | E13–E16 | Detail reservoirs, synthetic learned cuts, finite precision and alternative seams; no learned biological hierarchy. |
| [Rendezvous](families/rendezvous/README.md) | E17–E20 | Whole-input keys, exact overflow lookup, approximate nomination and subscriptions; broader recall/streaming remains. |
| [Tensor consumers](families/tensor/README.md) | E21–E24 | Sequence feature/relation/state consumers of CE math; Baseplane consumers remain host paths. |
| [Demand](families/demand/README.md) | E25–E28 | Certified fixture pull, opcode waves, debt/exploration and context wakeup; resident/learned/asynchronous variants remain. |
| [Compiler](families/compiler/README.md) | E29–E32 | Fitted teacher hardening, bank selection, typed rewriting and specialization guards; no biological training claim. |
| [Incremental](families/incremental/README.md) | E33–E36 | Exact repeat sharing, dirty cones, guarded memos and grammar reuse; persistent/distributed caches remain. |
| [Hypotheses](families/hypotheses/README.md) | E37–E40 | Parse alternatives, lossy sketches, query portfolios and targeted repair; held-out repair remains untested. |
| [Ports](families/ports/README.md) | E41–E44 | Synthetic regional solves, coarse correction, factor joins and alternative worlds; learned ports/P/R remain variants. |
| [Machine](families/machine/README.md) | E45–E48 | Texture responses, DP4A/butterfly, bounded transpose search and three residency routes; speed/crossover remains unmeasured. |

The [composition inventory](compositions/README.md) identifies five connected host
paths: C01 sequence effects, C03 variants, C04 nonlocal context, C05 compiled
learning and C06 ports/factors. C02/C07/C08/C09/C10 remain explicitly deferred.
The source-scoped [composition aggregate](compositions/validation/receipt.md) and
later [normal-entry Debug build](../../planning/baseplane_moonshot_bootstrap/results/final-build/receipt.md)
each passed 25/25 host tests. Nine Baseplane cards additionally passed root-assigned device
smokes: E01–E04, E18 and E45–E48. Twelve cards have compiled Baseplane sm_70 paths;
E26/E28/E38 remain compile-only. CE tensor E21–E24 GPU comparisons are separate
provider evidence, and do not imply Baseplane GPU consumers. See the additive
[GPU receipt](../../planning/baseplane_moonshot_bootstrap/results/gpu/qualified-checks.json).

## Build and run

Supply an explicit current CE provider source. The normal entry includes family
and composition tests; the aggregate can also be configured independently.

```sh
cmake -S experiments/moonshot -B /tmp/bp-moon-host \
  -DCE_MOON_SOURCE_DIR=/explicit/path/to/experiments/baseplane_moonshot \
  -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build /tmp/bp-moon-host -j1
ctest --test-dir /tmp/bp-moon-host --output-on-failure
/tmp/bp-moon-host/bp_moon_demo
```

Optional CUDA uses `BP_MOON_ENABLE_CUDA=ON` and an sm_70-capable toolkit (tested
CUDA12.9). CUDA13 removes offline compilation for pre-7.5 targets. Family device
executables are excluded from automatic CTest launch; use explicit resource
assignment, stream/capacity contracts and recorded runtime checks. Build commands
do not assign a GPU. The campaign used a shared four-slot build wrapper and one
compiler job per build; that temporary wrapper is not an installed dependency.

## Substrate and information contract

`Baseplane::moonshot` exports `include/` and consumes the explicit Cellerator
provider. `bp_moon/reference.hpp` retains sequence fixtures, validity, LUT/rank/count
helpers, toy finite effects, support union and exact dictionaries.
`bp_moon/source.hpp` retains source ID, contig, version, strand and origin; reverse
maps expect caller-oriented sequence. Invalid payloads never become canonical
bases. Emission uses caller-owned capacity and separates produced/stored/dropped.
Masks refer to declared coordinate domains; support union keeps gaps.

Exact source allows revisit; it does not make lossy counts, thresholds, sketches,
jets or latent responses lossless. Toy finite effects retain bounded state, and
floating evaluation has its stated approximation/rounding limits. Training here
uses small synthetic questions; no organism dataset or genome-wide learned model
was evaluated. There is no production ABI freeze or throughput result.

The [adoption receipt](../../planning/baseplane_moonshot_adoption/README.md) preserves
completed BitOp, CUDA-lab and documentation evidence. Representative experiments
cover selected migrated scope while general executor coverage, sanitizers,
performance policy, production adapters, biological evaluation and distribution
remain [Q01–Q05](../../planning/baseplane_moonshot_bootstrap/docs/06_LATER_QUALIFICATION.md).
Those are future obligations, not silently activated tasks. Current workflow state
belongs to native Todo authority.

The isolated sources lie outside the configured ctxpp source globs; canonical
inspection and actual compiler/test evidence were used. Generated context indexes
and historical receipts were preserved.
