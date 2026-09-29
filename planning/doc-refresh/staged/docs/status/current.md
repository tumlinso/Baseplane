# Current implementation snapshot

**Inspected:** 2026-09-29 · **Source:** `271843a2d2ceff434c97c8eb5fd13555567034c4`

This is a human-readable source snapshot, not a second task ledger. Revalidate it after a meaningful implementation change; the live project workflow holds task/claim state.

## Implemented facilities

The exact substrate supplies packed sequence, explicit validity/tail semantics, bounded local coordinates and global origins, scalar references, public exact count/emit entry points, and bounded predicate description/verification facilities. CUDA is optional. The isolated CUDA lab has completed its four synthetic experiments with a final combined-source replay and sanitizer receipts; see its current README and START_HERE guide.

## Experimental or deliberately bounded

BitLift, CarryFold, Rethread and Rendezvous are isolated experiments. The final selection retained one restricted ordered affine representation (CarryFold), with CUB as the faster measured executor. Rethread did not win its large fixture; BitLift loses arrangement/magnitude information; Rendezvous missed cross-warp peers and used oracle-assembled candidate packets. No public Baseplane API changed in that epic.

## Future intent and outstanding work

A trained genome-wide hierarchical embedding, general online context construction and scientifically demonstrated nonlocal representation remain future work. The older BitOp CUDA/backend tasks are not all complete merely because the independent lab finished. Do not require those old tasks to be completed for this documentation pass.

## Important qualifications

All lab weights and routing keys were untrained. Device-resident timings exclude allocation and transfer; some information probes are scalar fixtures. At the inspected head, the working tree showed active Todo projection updates and this documentation package; preserve them. No source edits were reported by git status. The inventory is a classification aid, not deletion authority.

See the [source map](../development/source-map.md) for current code paths, [results](../results/index.md) for measured evidence, and [design](../design/overview.md) for the scientific purpose that these facilities serve.
