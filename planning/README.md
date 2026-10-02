# Project planning

This directory contains reviewable source plans and planning notes for Baseplane
epics. Project Control and its Todo semantic store remain the live authority;
files here are inputs or durable planning records, not operational status.

Do not edit `todos.md`, `todo-status.md`, files under `todos/`, or
`.todo-orchestrator/state.snapshot.json` by hand. Those are generated
projections or recovery artifacts.

Current plan areas:

- `bitop/`: the deferred BitOP program retained for a later rewrite.
- Future intermediary epics should use their own subdirectory rather than
  modifying the deferred BitOP plan in place.

Moonshot campaign (adopted 2 October 2026):

- [Source package](baseplane_moonshot_bootstrap/START_HERE.md): preserved attachment.
- [Parallel plans](baseplane_moonshot_parallel/README.md): family/provider lane layout.
- [Adoption receipt](baseplane_moonshot_adoption/README.md): applied policies and replacement evidence.
