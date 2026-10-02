# Sequence-grounded compositions

BP-MOON-140 combines family mechanisms into executable source-to-query paths. The [catalogue](../../../planning/baseplane_moonshot_bootstrap/machine/compositions.json) proposes ten combinations. Five are implemented experimentally and passed the aggregate Debug host tests; the remaining five have explicit deferred scope below. Individual family probes do not establish these end-to-end combinations by themselves.

## Implemented experimental combinations

| Card | Local directory | Intended executable path | Evidence status |
|---|---|---|---|
| C01: sequence effects that answer questions | [sequence_effects](sequence_effects) | Exact sequence → finite and affine effect hierarchy → incoming-state query → source refinement | Debug host aggregate passed; see the local receipt for its semantic scope. |
| C03: one genome, many variants | [variants](variants) | Exact repeat reuse → counterfactual variant worlds → value and directory invalidation → guarded memo | Debug host aggregate passed; independent-world comparisons are scoped in its receipt. |
| C04: real nonlocal context | [nonlocal_context](nonlocal_context) | Input-derived keys → whole-input directory → relation tiles → contextual update wave | Debug host aggregate passed; its receipt records candidate construction and limits. |
| C05: learning compiles away | [compiled_learning](compiled_learning) | Floating teacher → discrete circuit → conditioned predicates → candidate nomination → exact verification | Debug host aggregate passed; its receipt records teacher/hardened comparison and verification. |
| C06: ports and factorized hypotheses | [ports_factor](ports_factor) | Regional response ports → multi-role factor join → counterfactual packet | Debug host aggregate passed; its receipt scopes the synthetic algebra and source support. |

The final Debug aggregate passed all 25 registered host tests, including these five combinations, against Baseplane `e5ddae14e6e690b68215fc1ec9586622bf54d6e9` and Cellerator `b2a65353d543a8d77d051be2d277d0c6ba4371dc`. [Aggregate validation](validation/receipt.md) records commands, source identities, binary hashes and logs. Local receipts describe which catalogue mechanisms each executable actually connects. The Cellerator provider includes the reviewed repairs and the later comparator-only change; this host check makes no GPU, timing or biological claim.

## Deferred combinations

| Card | Remaining implementation need |
|---|---|
| C02: residual-backed latent context | Connect adaptive boundaries, lifting reservoirs, higher-layer subscriptions, object compaction and dense mixing into one executable; measure omitted-detail effects and test selective decode back to exact source support. E13/E14/E20/E21 remain separate evidence until that path exists. |
| C07: dense possible states, sparse execution | Train/distill a bounded tensor-evaluated state vocabulary into a transition table, then consume it through finite regional effects and compare the hardened answers with the teacher. E24/E29/E05 probes alone do not establish distillation. |
| C08: hierarchy repairs its own blind spots | Join alternative seams with a query-specific portfolio, detect a real representation collision, and perform a targeted split/residual/predicate repair that changes the failing query. E16/E37/E39/E40 require a shared repair loop. |
| C09: precision follows a budget | Connect precision-plane fetches to uncertainty buckets and opcode cohorts, with bounded work queues, visible starvation debt and resumed refinement of unresolved work. E15/E27/E26 need one scheduler-facing sequence experiment. |
| C10: cheap role sketches, precise relations | Connect role sketches and multi-probe nominations to actual posting lookup, relation scoring and exact source-pair provenance. Record collisions, candidate recall and the richer verification path across E38/E19/E17/E22. |

Deferral records an unimplemented composition. It does not discard its family source or substitute a named design for executable evidence.

## Build contract

Each child directory owns its `CMakeLists.txt`, executable, CTest registration and receipt. Link `Baseplane::moonshot` and the required Cellerator family targets; declare extra local header paths in the child. Shared substrate and provider sources stay outside composition ownership.

This directory can be configured directly. Supply an explicit inspected Cellerator provider source path:

```sh
cmake -S experiments/moonshot/compositions -B /tmp/bp-moon-compositions-build \
  -DCE_MOON_SOURCE_DIR=/path/to/Cellerator/experiments/baseplane_moonshot \
  -DBUILD_TESTING=ON
cmake --build /tmp/bp-moon-compositions-build -j1
ctest --test-dir /tmp/bp-moon-compositions-build --output-on-failure
```

Standalone configuration imports the parent moonshot substrate, including its family tests and provider targets, then discovers `compositions/*/CMakeLists.txt`. When a caller already provides `Baseplane::moonshot`, it can use `add_subdirectory` on this directory directly. A global entry guard prevents recursive discovery if the imported parent also includes compositions. The parent moonshot build currently discovers families; adding compositions to that shared entry remains an integrator-owned change.

CPU tests cover bounded semantics. Compile/device checks, timings, compressed storage, biological evaluation and production integration require their own actual evidence. No build command in this README assigns or launches a GPU.

## Entry-point verification

On 2 October 2026, standalone configuration succeeded against the reviewed Cellerator integration workspace at provider commit `b72bfa3af4782303bc8630ff355d308b54f55ede`. A temporary parent configuration first imported the substrate, then included this composition directory twice; it also configured successfully. CTest JSON discovery in both builds registered 25 tests, with each of the five composition tests present exactly once. These were configuration/discovery checks; this entry-point worker did not compile or execute the composition targets. Final semantic results belong in the child receipts and root's integrated evidence.
