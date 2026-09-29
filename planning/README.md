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
