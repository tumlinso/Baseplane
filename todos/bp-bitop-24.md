

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-24: Property, adversarial, and differential test corpus

Task revision: `128`; current project revision is in `todo-status.md`.

## Objective
Fan in all CPU lanes into deterministic reusable fixtures and authoritative baseplaneBitOpCpuTest wiring.

## State
- Lifecycle: `blocked`
- Execution: `blocked_dependency`
- Parallel policy: `integration_exclusive`
- Result: `-`

## Next Action
Guard CMake, build deterministic generators/fixture schema, wire baseplaneBitOpCpuTest, run CPU-only acceptance, reach BITOP_CPU_REFERENCE_READY.

## Ownership
- `exclusive`: `CMakeLists.txt`
- `exclusive`: `tests/bitop`
- `exclusive`: `tests/bitop_fixture.hh`
- `read`: `include`
- `read`: `src`
- `read`: `tests/emit`
- `read`: `tests/segment`
- `read`: `tests/seq`

## Dependencies
- `task`: `BP-BITOP-20`
- `task`: `BP-BITOP-21`
- `task`: `BP-BITOP-22`
- `task`: `BP-BITOP-23`
- `task`: `BP-BITOP-25`
<!-- todo-orchestrator:v2-managed:end -->
