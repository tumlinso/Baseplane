# Unconventional Volta consumers: E45–E48

This BP-MOON-130 family contains four concrete host experiments and one compiled
sm_70 executable that exercises their GPU counterparts. Host semantics passed;
GPU execution remains assigned to the root. No timings or throughput wins are claimed.

| Card | Disposition and entry | Distinction |
| --- | --- | --- |
| E45 | Implemented experimental fixed response surfaces; `response`, `coordinates`, CUDA texture configuration and CE `texture_response` consumer | Explicit nearest and interpolated textures are compared with direct arithmetic. Surfaces are untrained bilinear/discontinuous fixtures; fitting a learned table remains later scope. |
| E46 | Implemented fixed signed-byte routing and invertible butterfly consumers; `features`, CE `packed_dot` and `butterfly32` | DP4A computes four signed widened products; the butterfly is an independent order-sensitive floating transform. Neither is a binary tensor operation. |
| E47 | Implemented bounded search and two compiled 4×4 bit-transpose survivors; `synthesize`, `transpose_shift`, `transpose_table` | Eight candidates reduce to a shift/mask network and a constant-table mapping, both exhaustively checked over all 65,536 sixteen-bit inputs. Selected native disassembly is saved. |
| E48 | Implemented three residency routes; `fused`, `staged`, `materialized`, CUDA `gc_fused`, `gc_staged`, `make_planes`/`gc_planes` | Exact GC counts stay fixed while predicates are recomputed, shared-staged with ordinary software prefetch, or globally materialized. GPU semantic comparison is authored and compiled, pending execution. |

## Grounding and numerical meaning

`Input` retains the installed packed sequence, validity and `SourceMap` identity,
contig, origin, version and strand. Outputs are indexed by packed word or original
lane. Their source region is recovered from that index and retained SourceMap;
invalid bases and tail slots remain explicit in the retained input validity.
No GPU representation replaces this source information.

E45 coordinates are fractions of valid C and G counts scaled into a 4×4 surface.
They lose order and most exact detail; the packed source remains available for
revisits. This is a floating response consumer, never an exact motif/validity gate.
Coordinates in host arithmetic use integer texel centers 0–3; unnormalized texture
coordinates add 0.5. Both axes clamp, the channel/read mode is float, and filtering
is explicitly point or linear. The host oracle uses mathematical interpolation;
hardware fractional precision is allowed an explicit 0.04 absolute fixture bound
for linear fetches. Nearest fetch and direct arithmetic use 1e-5 tolerance.
The probe includes a bilinear surface, a sharp discontinuity, boundaries and
sequence-derived coordinates. Table allocation/upload/configuration are explicit.

E46 byte features are counts of A/C/G/T minus four, yielding exact signed bytes in
[-4,28]. Scale is 0.25 and zero point is zero; fixed integer weights are [2,-1,3,-2].
The byte score converts to physical units by multiplying by the feature scale.
These exact fixture counts introduce no additional rounding error, but discard
position. Each product is widened before summation; four products of arbitrary
signed bytes fit int32. The separate 32-lane butterfly preserves order-sensitive
linear information and inverts through another application divided by 32.
Invalid lanes are zeroed and retain their validity mask separately. Generic device
math is reused from Cellerator's `ce_moon/volta.cuh`; local scalar calculations are
comparison oracles for these sequence consumers, rather than another numerical API.

E47 transposes four rows of four bits, changing storage orientation without losing
any of the sixteen bits. The search rejects six alternatives and fully verifies
both survivors. Exhaustiveness is limited to this declared sixteen-bit domain.
Shift masks and a derived nibble table have different register/constant-memory
costs. Native SASS has 32 static instruction slots for each, including compiler
control/padding; equal counts do not imply equal runtime. Registers differ: 10 for
the network, 14 for the lookup. Disassembly was explicitly requested by E47;
collection was limited to its two survivor functions.

E48 computes `popcount((lo XOR hi) AND valid)` per packed word. Host variants agree
on 530 bases with invalid entries, a 17-word partial tile and an all-invalid case.
The CUDA probe uses 9001 bases (282 words), crossing both 128-word software-buffer
and 256-word block boundaries. Ordinary loads prefetch the next tile into registers;
explicit block barriers publish the alternate shared buffer. Every thread reaches
the barriers, including neutral tail threads. There is no cp.async/TMA assumption.

## Capacity and accounting

These are fixed-size probes. Host vectors and device allocations have exact input
cardinalities; output capacity equals the word/query count, so no output truncation
is used. Neither kernels nor consumers allocate device memory. Harness allocation,
sequence packing, table creation, uploads and downloads are visible and included
in the runnable path, without timing them. Default-stream ordering and synchronous
copies establish producer completion; each launch status is checked. The CE warp
butterfly uses one fully participating warp.

For E48, fused input traffic is 8 bytes of packed sequence plus 4 validity bytes
and 4 output bytes per word, before caching. The materialized route adds 8-byte
plane writes and reads plus another launch; repeated questions may reuse planes.
The staged route adds shared writes/reads and barriers. It reserves 3072 shared
bytes per block. Ptxas reports zero spills for these kernels. Fused and plane
construction use 38 registers; staged uses 22; the resident-plane consumer uses 12.
This is a resource observation, not an occupancy or performance claim.

## Reproduce and evidence

```sh
cmake -S experiments/moonshot -B /tmp/bp-moon-machine \
  -DCE_MOON_SOURCE_DIR=/home/tumlinson/Cellerator/experiments/baseplane_moonshot \
  -DBP_MOON_ENABLE_CUDA=ON \
  -DCMAKE_CUDA_COMPILER=/opt/nvidia/hpc_sdk/Linux_x86_64/26.1/cuda/12.9/bin/nvcc \
  -DCMAKE_CUDA_HOST_COMPILER=/usr/bin/g++
python /tmp/moonshot_build_slot.py cmake --build /tmp/bp-moon-machine \
  --target bp_moon_machine_smoke bp_moon_machine_cuda_smoke -j1
ctest --test-dir /tmp/bp-moon-machine -R '^bp_moon_machine_smoke$' --output-on-failure
/tmp/bp-moon-machine/families/machine/bp_moon_machine_smoke
```

Only the root may launch the authored GPU probe with an assigned device:
`bp_moon_machine_cuda_smoke --run DEVICE`. It is excluded from CTest. The worker
made no GPU launch and therefore claims no measured texture precision or device
correctness. Compiler: GCC 13.3.0 / CUDA 12.9.86, target sm_70.

Receipts in `results/`:

- `host-ctest.txt` and `host-smoke.txt`: one passing test, four host dispositions.
- `compile.txt`: successful final CUDA compile and actual ptxas resources.
- `compile.initial-failure.txt`: first compile errors, repaired locally by using
  CUDA's `uint2` spelling and selecting the feature byte array for packing.
- `e47.sass.txt`: selected native disassembly from CUDA 12.9 `cuobjdump --dump-sass`.
- `machine-evidence.json`: exact executable SHA-256, selected resource lines and
  static instruction counts; `gpu_executed` remains false.

Later work includes root-run device correctness, texture error beyond these
fixtures, resource scaling, fitting response surfaces or weights, and residency
crossover benchmarks. All four cards have a concrete experimental implementation;
none is declared scientifically validated or optimized.
