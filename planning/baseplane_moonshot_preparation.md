# Moonshot bootstrap preparation

Prepared 2 October 2026. This records the preparation pass, not live task state.
The subsequent user-authorized application is recorded in the
[adoption receipt](baseplane_moonshot_adoption/README.md).

## Scope and intent

The request was to extract, understand and prepare the bootstrap, including broad
implementation parallelism. The attachment's directions to implement, replace
the BitOp run, amend policy and adopt Cellerator tasks are proposals for a later
execution step. They have not been executed or treated as new user instructions.
The user's subsequent message explicitly authorized revising the bootstrap for
parallelism. Derived plans are prepared separately from the original package;
this authorization does not itself activate implementation or supersession.

The research goal is learned hierarchical sequence representation whose masks,
effects, candidates and refinement requests organize computation during encoding.
Exact sequence and provenance remain available; neither source recovery nor a
synthetic fixture establishes a lossless embedding or biological usefulness.
The workshop explores multiple concrete mechanisms and their compositions.
Cellerator owns shared numerical and learned operators from inception; Baseplane
owns sequence grounding and sequence-facing experiments.

## Extracted package and checks

- [Original package](baseplane_moonshot_bootstrap/START_HERE.md): 117 files,
  unchanged contents; archived permission bits restored.
- Archive SHA-256: `dabe36160ecb8535ccb8eb15c0fba43a18eaa1c71235b6f983e8597fe2a9efc3`.
- Byte comparison and ZIP CRC passed. The inspected package checker passed with
  manifest verification: 116 payload hashes, 22 JSON files, 48 experiments,
  12 families, 10 compositions, 16 Baseplane records, 7 Cellerator records and
  33 old-task dispositions. See [check receipt](baseplane_moonshot_preparation_evidence/package-check.json)
  and [archive comparison](baseplane_moonshot_preparation_evidence/archive-byte-verification.json).
- No host/CUDA seeds, benchmarks or training were run during preparation.
  Supplied receipts remain historical attachment evidence.
- Keep future build output outside the package. Its checker excludes `build-host`
  but not the default CUDA `build-cuda` directory.
- Adoption examples and machine-plan source references use
  `planning/bp-moonshot-20261002`. Before adoption, prepare derived payloads using
  the actual `planning/baseplane_moonshot_bootstrap` path and revalidate them, or
  reconcile the installation path. Native validation did not detect these missing
  source references. Preserve the original package bytes.

## Fresh authority observations

Local worktree HEAD and registered Baseplane HEAD both resolve to
`f4e9986607becd8528f42c36f0890bd7dd8f4a0d`. Project Control observed Todo revision
216, active run `compat-v2`, and explicitly blocked/deferred `BITOP-00`, with no
active claims in the returned context. The registered worktree differs from this
attachment worktree; matching commits do not register the extracted files there.

A research subagent submitted the full original Baseplane plan to the native
nonmutating `plan_preview(mode=validate)`. It returned `valid=true`, all 16 tasks
as additions, no dependency/scope/interface errors, `mutation_guard=unchanged`,
base revision 216, and `authority_to_apply=false`. Reported plan digest:
`42029b08f45ec2a4750e51033ee1b60377e477d636a2873811220ca788c56cf0`.
These are a digest of the tool result, not a persisted raw validation receipt.

The corresponding full Cellerator preview returned `internal_error` with
`bounded_read_failed` at 13:01:45 UTC. Cellerator native validity, current claims
and adoption readiness remain unverified by that preview. A separate coordination
read succeeded at revision 7474, head
`47bd95a21d5143d2293dedbdaa35b6f1d97dda3c`: selected run `CE-ML2-RUN-V1`,
also active `CE-PTR-RUN` and `CE-AMP-RUN-V1`, without surfaced dispatches or
pending integration. This is not an exhaustive conflict audit. The preview error
does not show that its plan is invalid. No plan, policy, lifecycle state or run
membership was changed.

## Parallel implementation structure

The original task graph is already broad; its supplied execution lane is serial.
The derived plans replace that lane with 14 Baseplane lanes and 6 Cellerator
lanes: exclusive foundation, isolated family/provider lanes, and exclusive
integration. Tasks, dependencies, scopes and task parallel policies are preserved.
Each task appears in exactly one lane. The final Baseplane payload passed native
validation at revision 216; the final Cellerator preview again returned
`bounded_read_failed`. See the derived directory's evidence for exact receipts.

```text
BP-010 -> BP-020 ... BP-130 (12 independent family tasks) -> BP-140 -> BP-150
CE-010 -> CE-020 ... CE-050 (4 numerical providers)         -> CE-090
```

Here prefixes abbreviate `BP-MOON` and `CE-MOON`. The all-family barrier before
BP-140 is in the supplied plan. Starting composition tasks earlier needs an
explicit supported dependency/workflow revision; ready components alone do not
override that barrier. Provider receipts cross repository boundaries separately
from local task dependencies.

| Workstream | Baseplane family tasks | Main Cellerator dependency |
|---|---|---|
| Exact geometry and finite effects | 020 masks; 030 state machines | Shared numerical effects split during foundation |
| Hierarchy and retrieval | 050 adaptive refinement; 060 global candidates | CE-020 for relevant numerical refinements |
| Demand and reuse | 080 query execution; 100 incremental/repeat reuse | Provider-specific operations only |
| Representation exploration | 110 multiple interpretations; 130 Volta paths | CE-040/030 where learned/tensor operators are used |
| Continuous effects | 040 | CE-020 |
| Tensor representations | 070 | CE-030 |
| Learned cheap machinery | 090 | CE-040 |
| Ports and counterfactuals | 120 | CE-050 |

The last four rows are primary producer/consumer pairs. Additional consumers are
listed in [cross-authority.json](baseplane_moonshot_bootstrap/machine/cross-authority.json);
the table is not a complete provider dependency list.

### Foundation before fan-out

1. Reconcile the owning tasks, worktrees and policy, then establish the small
   common substrate under BP-010 and CE-010.
2. Split the three mixed seed files once according to
   [seed ownership](baseplane_moonshot_bootstrap/machine/seed-ownership.json).
   Use an explicit provider build target/path, without hardcoded sibling paths.
3. Agree only the immediate seams: shapes/axis meaning, state and weights,
   validity/source mappings, precision/loss, buffers, streams and capacity.
   Each provider returns a minimal input/output fixture and source digest.
4. Give one integration owner common headers, root build files and seed splitting.
   Family workers own their assigned family directory and local tests/build files.
   Shared changes return as integration requests. Avoid a universal experimental ABI.

### Practical concurrency

Use supported run/lane configuration for first-class concurrent tasks; a
`parallel_safe` family label does not make the current serial lane dispatch them
concurrently. Subagents remain bounded by the root's active claim and assigned
ownership. Do not put twelve sibling tasks beneath an unrelated foundation claim.

After foundation, a maximum-width option is root + four Cellerator providers +
four Baseplane workers for masks, state machines, hierarchy and candidate discovery
(nine total slots). Use fewer workers if provider contracts are still moving.
Rotate completed slots into demand, reuse, representation and machine families;
activate paired consumers as usable provider receipts arrive. Fixture development
can overlap provider development, but a scalar oracle is not provider delivery.

Prefer direct bounded implementers initially. A Cellerator parallel head becomes
useful only if coordinating its four providers becomes substantial; it consumes
one slot and must replace a worker in this allocation. The root retains ownership
decisions, cross-repository coordination and final integration.

CPU work and CUDA compilation can overlap within host resource limits. GPU
execution needs assigned resources; benchmark timing intervals must be isolated.
Agent concurrency is not permission for simultaneous timing runs.

### Integration strategy

Prioritize several end-to-end paths, for example C01 effects answering queries,
C03 repeat/variant reuse and C04 real nonlocal context. These exercise different
failure modes. Develop family fixtures with these connections in mind; formally
dispatch BP-140 only when its dependencies are satisfied or validly revised.

Track authored, compiled, run and compared separately. Close family work with
concrete code or individual, reasoned card dispositions. Essential semantic and
capacity checks support this workshop; production benchmarks, large training,
ABI stabilization and multi-GPU qualification remain the separately proposed
Q01–Q05 work. Existing required workflow gates still apply until validly amended.

## Bootstrap sequence to perform when execution is requested

1. Refresh both authorities and current claims; bind the actual worktrees.
   Preserve dirty work and authoritative state before mutation.
2. Resolve Cellerator preview access and validate its complete companion payload.
   Reconcile embedded installation paths in derived payloads. Capture fresh full
   native receipts/diffs for both exact adoption payloads at adoption time.
3. Review the 33-record old-to-new map and policy amendments against current
   invariants. Policy-intent JSON is not itself an applied native amendment.
4. Apply through the supported front door, then use the signed Baseplane
   supersession route. Preserve completed lab/docs evidence and deferred
   qualifications. Adopt Cellerator independently without replacing ML2.
5. Verify replacement and policy postconditions before dispatch. Configure the
   parallel lanes under supported authority, then claim exact ready tasks.

The original Baseplane native plan validation passed. See the
[parallel adoption candidate](baseplane_moonshot_parallel/README.md) for revised
plans and their validation status. Paired adoption, policy reconciliation and
supersession remain future bootstrap work. No implementation has been activated.
