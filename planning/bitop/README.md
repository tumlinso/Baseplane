# Deferred BitOP plan

The existing BitOP plan was deferred by user direction on 2026-09-29 so that
an intermediary epic can run first. Its complete plan source is preserved as
`bitop-plan.json`.

All 24 nonterminal BitOP tasks remain nonterminal and blocked. They carry the
tags `deferred` and `bitop-rewrite-pending`, and their next action explicitly
forbids claiming, activation, or implementation until both conditions hold:

1. the user explicitly authorizes resumption; and
2. a replacement BitOP plan has been reviewed and applied through Project
   Control.

Completed and superseded historical tasks, frozen interfaces, evidence,
dependencies, and scopes are retained. Three legacy command-shaped gates
(`BP-BITOP-50-MATRIX`, `BP-BITOP-50-NSYS`, and
`STACK-BITOP-51-FOUR-GPU`) were normalized from `benchmark` to `command`
because the current validator requires benchmark gates to declare a numeric
metric threshold. Their commands, required status, locks, and resources are
unchanged. Do not use this plan as the starting point for the intermediary
epic and do not manually edit generated Todo projections.

The stored plan uses schema v3 with an empty `runs` list, so validating or
reapplying it does not create a new BitOP workflow run. The existing
`compat-v2` run created while recording the deferral remains active with a
queued lane, while the Todo frontier reports all unfinished members blocked
and no ready tasks. Project Control has no nonterminal operation that simply
deletes that run; retire or replace it only when the intermediary epic or
rewritten BitOP program supplies an explicit successor.
