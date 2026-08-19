

<!-- todo-orchestrator:v2-managed:start -->
# BP-BITOP-35: Prepared runtime, residency, backend planner, and CUDA fan-in

Task revision: `16`; current project revision is in `todo-status.md`.

## Objective
Provide allocation-explicit prepare/query/bind/run surface, integrate all CUDA families, and publish executable Baseplane BitOp v1.

## State
- Lifecycle: `planned`
- Execution: `ready`
- Parallel policy: `integration_exclusive`
- Result: `-`

## Next Action
Guard shared seams, integrate CUDA lanes, wire baseplaneBitOpCudaTest, expose required_scratch/prepare/bind/run and inspectable backend choice, run CPU/CUDA/sanitizer fan-in, reach both executable checkpoints.

## Ownership
- `exclusive`: `CMakeLists.txt`
- `exclusive`: `include/Baseplane/dna2.hh`
- `exclusive`: `include/Baseplane/runtime/prepared_predicate.cuh`
- `exclusive`: `include/Baseplane/runtime/resident_sequence.cuh`
- `exclusive`: `include/Baseplane/seq/dna2.cuh`
- `exclusive`: `src/runtime/prepared_predicate.cu`
- `exclusive`: `tests/runtime/test_prepared_predicate_cuda.cu`
- `read`: `include/Baseplane/emit`
- `read`: `include/Baseplane/segment`
- `read`: `include/Baseplane/seq`
- `read`: `src/emit`
- `read`: `src/segment`
- `read`: `src/seq`
- `read`: `tests/bitop_fixture.hh`

## Dependencies
- `task`: `BP-BITOP-30`
- `task`: `BP-BITOP-31`
- `task`: `BP-BITOP-32`
- `task`: `BP-BITOP-33`
- `task`: `BP-BITOP-34`
<!-- todo-orchestrator:v2-managed:end -->
