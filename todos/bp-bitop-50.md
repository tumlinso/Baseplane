

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-50: Benchmark matrix and measured backend selector

Task revision: `16`; current project revision is in `todo-status.md`.

## Objective
Measure crossover matrix and publish replaceable device-fact-versioned backend policy with same-build correctness checks.

## State
- Lifecycle: `planned`
- Execution: `ready`
- Parallel policy: `serial`
- Result: `-`

## Next Action
Guard shared benchmark/CMake seams, wire baseplaneBitOpBench, run serialized matrix/checksum/sanitizer/nsys evidence, set measured policy decisions, publish selector inputs and exact commands.

## Ownership
- `exclusive`: `CMakeLists.txt`
- `exclusive`: `bench/bitop/bitop_bench.cu`
- `exclusive`: `bench/bitop/bitop_bench.hh`
- `exclusive`: `bench/results/bitop-v1`
- `exclusive`: `docs/BITOP_PERFORMANCE.md`
- `exclusive`: `include/Baseplane/runtime/backend_selector.hh`
- `exclusive`: `src/runtime/backend_selector.cpp`
- `read`: `bench/seq`
- `read`: `docs/bitop_baseline_evidence.md`
- `read`: `include`
- `read`: `src`
- `read`: `tests`

## Dependencies
- `barrier`: `BITOP-B3`
<!-- todo-orchestrator:v2-managed:end -->
