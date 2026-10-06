# Repository work contract



Read the [design](docs/design/overview.md) for intent, [current snapshot](docs/status/current.md) for its dated implementation boundary, and [source map](docs/development/source-map.md) for entry points. For an edit, the current scoped task and inspected source—not a historical plan or README completion sentence—determine what exists and what is authorized.

## Operate within the current authority

Use the installed Project Control/Codex front door and this repository's Todo authority. Take one bounded outcome, inspect only what it needs, preserve other claims/dirty work, and finish with actual evidence. Delegate bounded research, implementation, tests and review to configured Codex subagents when useful; the owner retains task lifecycle and final acceptance. Do not use Project Control `delegate_task` or `local-coding-worker` for ordinary delegated implementation; local workers are reserved for Project Control observers. Never edit SQLite, generated Todo views, recovery snapshots or generated context indexes by hand. Keep managed workflow blocks intact.

## Preserve the architecture while changing the implementation

- Preserve exact sequence identity, coordinates, strand, validity and provenance; do not treat invalid packed payloads as biology.
- Keep online hierarchical floating-point representation as the goal. Packed predicates are a foundation, not the whole project; no preparatory oracle may supply the hierarchy being claimed as learned.
- Measure information loss and candidate/routing/revisit costs. A source map is not proof of a lossless embedding.
- Keep CPU reference correctness and optional CUDA usability; caller-owned buffers, explicit streams/capacity/overflow and target capability remain visible.
- The isolated CUDA lab is experimental, not a production interface. Preserve its evidence and completed negative decisions. Cellerator general numerical ownership and Baseplane sequence-native experiments must not be conflated.

## Software ports and design probes

During active library development, any explicitly user-authorized software
port or design probe with a scoped Project Control task carries standing
permission for additive changes to Baseplane when evidence shows an
intrinsically sequence-grounded primitive is needed, without renewed
permission. Keep
additions general, clean, and maintainable, with explicit contracts and
validation; use small architectural cleanup only to resolve demonstrated
friction. General numerical, state, graph, relation, statistical, and
iterative computation belongs in Cellerator; Baseplane owns only intrinsically
sequence-grounded functionality.

Preserve native computation by default. An optional, opportunistic FP16 Tensor
Core mode is the only permitted deliberate numerical deviation. Compare it
with upstream/native computation; a higher-precision reference may supplement
that comparison. Prefer FP32 accumulation where supported, and declare and
qualify actual accumulation and output policy. Use the mode only within its
demonstrated numerical envelope, retaining native fallback for unsupported or
unsafe regimes.

When a basic reusable primitive appears missing or insufficient, report this
promptly to the user. Include the concrete computation and evidence, current
library capability and gap, proposed owner and general contract, and
downstream impact. Discuss major API, granularity, ownership, or architectural
choices with the user before committing to them.

External compatibility is not an absolute constraint at this stage. A reviewed internal move may change names/interfaces if it improves development; repair real sibling consumers and relevant tests together. Frozen contracts and overlapping active work still require their owner's explicit reconciliation. Do not use a documentation task to rewrite numerical behavior, introduce another planner, or complete unrelated old epics.

## Validate what changed

Use the [development guide](docs/development/start.md) for the current commands. Run affected source/build/tests after code moves. For prose-only changes check links, status boundaries and rendered pages. Keep scientific claims scoped; record what was not run. Benchmarks require the existing assigned resources and clean timing interval. Include setup/transfer/routing when the claim needs them; no benchmark is launched by document generation.

Current work belongs in live status tools; experimental findings in [results](docs/results/index.md); durable rationale in design/development docs; superseded plans in the archive. Do not recreate the same mutable status table in all four places.
