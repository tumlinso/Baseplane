# Adopted sequence and representation seam

Base: `5de838c09a7e1dd5795b0fdd4774862f0c846ebc`.

`Baseplane::seq` keeps exact packed sequence validity, chunk ownership and coordinates,
version 1 predicate preparation, and caller-owned event/capacity contracts. It builds
and installs independently of Cellerator. Sequence strand remains in event flags;
source identity and generation remain descriptor metadata. Existing source interfaces
are retained for real Cellerator consumers.

`baseplane_add_representation_target(target COMPONENTS Cellerator::component ...)`
adds an opt-in interface target after both providers have been loaded. It links
`Baseplane::seq` and only the explicit components, checks that each exists, and refuses
direct umbrella targets. It never loads the Cellerator root or changes `Baseplane::seq`.
The module ships with the Baseplane package. The caller owns its target and export
policy; this helper alone introduces no representation implementation.

## Parallel leaf boundaries

| Leaf | Retained sequence source | Numerical provider decision |
|---|---|---|
| SEQ | `include/Baseplane/seq`, `src/seq` | No numerical dependency required |
| HIERARCHY | `experiments/moonshot/families/automata`, `effects`, `hierarchy` | Select exact required CE effects/operations; preserve ordered arguments and source generations |
| INDEX | `experiments/moonshot/families/rendezvous`, `demand` | Select CE operations for numerical execution; keep candidate nomination distinct from identity |
| REUSE | `experiments/moonshot/families/incremental`, `hypotheses` | Preserve source/world/model guards and dependency invalidation |
| LEARNING | `experiments/moonshot/families/compiler`, `tensor`, `ports` | Select CE differentiation/training components; learned fitting and biological evaluation remain separate |
| BUILD | package/module and all real consumers | Export selected components without a CE-to-BP-to-CE umbrella cycle |

Current native CE target names confirmed by CE adoption: `Cellerator::native_foundation`,
`Cellerator::indexed_mechanism`, `Cellerator::local_differential`,
`Cellerator::native_numeric`, `Cellerator::product2`. Availability depends on the actual
CE build/toolchain. CE package qualification is BUILD work. The current host fixture
uses the existing `Cellerator::moonshot_effects` provider and the finished C01 consumer.
That historical selected provider still links its header substrate; this adoption does
not claim the old experimental graph has been refactored into native components.

`source-dispositions.json` expands the package's seed map with all 48 current BP card
entries, exact evidence, loss/revisit limits, remaining variants, 33 historical BitOp
dispositions, composition limits and qualification obligations. The applied supersession
receipts retain historical authority; this map does not retire or reopen old tasks.
All unfinished Q01–Q05 requirements survive in the map and designated successor owners.

Fresh host checks cover actual compiled sequence calls (invalid/tail payload, coordinates,
ownership, strand, capacity), the current C01 exact-to-affine/source-refinement consumer,
and existing predicate preparation and validity tests. They do not qualify CUDA,
performance, genome-scale learning or production representation APIs.

## Re-fetch and gates

```
python3 planning/integrated-substrate-v1/adoption/check_dispositions.py
cmake -S planning/integrated-substrate-v1/adoption -B /tmp/bp-is1-adopt-host -DCE_MOON_SOURCE_DIR=/home/tumlinson/Cellerator/experiments/baseplane_moonshot -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/bp-is1-adopt-host --target bp_adoption_seq_consumer bp_adoption_effect_consumer baseplaneBitOpContractTest baseplaneDna2ValidityTest -j 2
ctest --test-dir /tmp/bp-is1-adopt-host -R '^bp_adoption_' --output-on-failure
/tmp/bp-is1-adopt-host/baseplane/baseplaneBitOpContractTest
/tmp/bp-is1-adopt-host/baseplane/baseplaneDna2ValidityTest
```

Root owns live activation reconciliation, task gate binding, commit and final acceptance.
Merge this patch before creating dependent worktrees. Use the root's fresh activation
receipt for claims/pending patches/frozen interfaces; source checks here are additive.
