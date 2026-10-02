# Logic family: masks as executable geometry

Four experimental mechanisms implement BP-MOON-020. Build through the shared
moonshot CMake project; this directory owns no production Baseplane interfaces.

| Card | Concrete artifact | Disposition |
|---|---|---|
| E01 | `Circuit`, two typed validity-masked LUT gates; CUDA LOP3 circuit | Implemented experimentally |
| E02 | `Evidence`, carry-plane modulo and saturation counters, Boolean threshold; CUDA carry counter | Implemented experimentally |
| E03 | `grammar`, adjacent-word motif grammar and reverse-complement coordinate transform; CUDA ACG funnel grammar | Implemented experimentally |
| E04 | `route`, rank/select query intersection and child-to-parent support composition; CUDA POPC/select routing | Implemented experimentally |

`logic.hpp` supplies host mechanisms; `smoke.cpp` compares their semantics.
`cuda.cu` supplies four independent bounded kernels and a tiny optional device
smoke. Host reverse-strand and saturating variants are implemented; their CUDA
variants remain future work. E03 demonstrates adjacency; arbitrary spacing and
exclusion grammars remain extensions. No learning run created the LUT constants:
`parameter_version` identifies the fixed truth-table fixture. This family claims
neither trained representation nor biological efficacy or throughput gain.

Masks use original forward word coordinates. Their declared domain contains
source ID, contig, source version and word; intersection and circuit composition
reject different domains or validity. Circuit truth-table constants and parameter
version are retained; outputs omit floating magnitudes. Exact source sequence is
kept for revisit. Counting stores four-bit votes and sticky overflow, so modulo
counts alone lose high bits; saturation has separate semantics and overflow.
Threshold outputs discard count magnitude. Motif operators require every base to
be valid, including tail and halo, and preserve source ID/version/strand in emitted
anchors. A reverse hit uses its high original-coordinate anchor. Palindrome
policies either retain both strands or prefer one forward anchor. Rank addresses a
compact descriptor only inside its parent support; support composition preserves
gaps and rejects child bits outside the parent. Rank is never a global identity.

Host result vectors grow with the bounded fixture output. CUDA kernels allocate
nothing, accept caller-owned nonaliasing arrays and run on an explicit stream.
For query routing, provide 32 coordinate and rank slots per input word; only the
first `popcount(parent & question)` slots are initialized. Grammar inputs are
already validity-masked base planes; the last word's absent halo is zero. Kernel
input masks must share the same validated source domain. CUDA masks contain no
provenance by themselves: retain the host domain sidecars. Test harness indices
are tiny; these kernels are not a generalized whole-genome launch API. The harness
synchronizes its stream before consuming output and checks each launch/runtime
operation. Counting has explicit overflow; other output sizes are bounded by one
word or 32 slots per word.

The computations differ: LUT evaluation executes a local Boolean circuit;
counters accumulate graded votes; grammar transforms relative position and
orientation; rank/select routes sparse descriptors. Exact routing preserves
selected coordinates, while LUT and threshold summaries lose task information.
Mask packing, construction and revisit costs have not been timed.

Source support: E01/E02 use the supplied reference LUT/counter conventions,
E03 uses exact fixture planes and validity plus an explicit original-coordinate
transform, E04 uses reference rank/select. Catalogue citations S02, S03, S15, S16
and S22 remain inspiration; no new literature or instruction-performance claim is
made. NVIDIA 12.9 compilation establishes target acceptance of these paths.

## Reproduce

```sh
cmake -S experiments/moonshot -B /tmp/bp-moon-logic-build \
  -DCE_MOON_SOURCE_DIR=/home/tumlinson/Cellerator/experiments/baseplane_moonshot \
  -DBP_MOON_ENABLE_CUDA=ON \
  -DCMAKE_CUDA_COMPILER=/opt/nvidia/hpc_sdk/Linux_x86_64/26.1/cuda/12.9/bin/nvcc
cmake --build /tmp/bp-moon-logic-build --target bp_moon_logic_smoke bp_moon_logic_cuda -j1
ctest --test-dir /tmp/bp-moon-logic-build -R '^bp_moon_logic_smoke$' --output-on-failure
```

After the controller assigns a GPU, it may run
`/tmp/bp-moon-logic-build/families/logic/bp_moon_logic_cuda --run DEVICE`.
No GPU was launched during family implementation. See `evidence.json` for actual
compile and run status.
