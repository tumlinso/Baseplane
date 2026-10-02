# Sequence consumers for learned cheap programs

This family implements BP-MOON-090 with the Cellerator CE-MOON-040 provider.
`compiler.hpp` contains sequence interpretation, exact predicates, packed
routing, source witnesses and version guards. Numerical fitting, bank scoring,
Boolean rewriting and circuit specialization remain in Cellerator.

The provider consumed here is commit
`b16295a56e02d98b299bc2c828e1b69aeb7ea58d`, header
`experiments/baseplane_moonshot/families/learning/learning.hpp`, SHA256
`a4fa4c3312fb26492a6cb217b4aad91aa17be95a4e43ff7fe442a9727d4ba8f3`.
Use an explicit `CE_MOON_SOURCE_DIR` exposing
`Cellerator::moonshot_learning`; the family links that target directly. No
numerical provider copy or sibling-path default is installed here.

The four experiments ask different questions:

- **E29:** learn a synthetic sequence triplet question: current base is GC and
  previous base is A, or next base is T. Eight real training triplets supply
  Boolean targets to the CE relaxed teacher. Features at inference depend only
  on the supplied sequence. The hard LUT is applied to 32-position bitplanes,
  followed by complete triplet-validity masking. The held-out fixture covers all
  64 canonical triplets plus lowercase/invalid payloads, and a separate fixture
  checks a 31/32 word seam and reverse source coordinates. Continuous teacher
  confidence is lost during hardening; exact sequence remains for source replay.
- **E30:** two synthetic state-to-bank models choose between the immutable exact
  AC and GT predicates. The same sequence returns different, source-grounded
  motif anchors in the two states. Learned bank choice is a routing hypothesis;
  it changes the requested question rather than a motif's exact meaning.
  Bank count is fixed at two, without allocating a bank per possible state.
- **E31:** the source question `GC & (previous_A | next_T)` has a distributed
  three-instruction form and a two-instruction equivalent. CE searches its
  bounded Boolean space; Baseplane binds an explicit coordinate-domain token,
  applies the extracted truth table to valid feature planes and emits exact
  source witnesses. Held-out discrete comparisons use a separate scalar
  sequence question. Strict floating reassociation remains forbidden. Affine
  rewrites, layout search, register/movement costing and SASS inspection are
  later variants; this implemented branch estimates Boolean instruction count.
- **E32:** a regional circuit specializes the E29 teacher, preserving separate
  model and circuit versions. Weight/query/state/source versions and the full
  source identity (ID, contig, origin, length, strand) determine reuse. Fresh
  features are read even when the circuit is reusable. Guard failure executes
  the current CE teacher, records source revisits and sets emitted circuit
  provenance to zero. Changing weights produces different answers rather than
  silently reusing the cached circuit. Re-specialization after repeated use is
  a later policy; this fixture exposes applicability and fallback directly.

`Hit` preserves source ID, contig, source version, strand, exact anchor and two-
 or three-base support coordinates, plus weight/circuit/query/state/bank
provenance. The caller supplies oriented sequence and a matching `SourceMap`;
reverse mapping transforms coordinates, with no implicit strand reinterpretation.
Domain tokens must identify the same coordinate space for all expression inputs.
A caller must advance source and model versions when their contents change.

Every emitting API uses a caller-owned output and capacity. It reports produced,
stored and dropped counts, accepts zero-capacity dry runs, skips invalid bases
and rejects mismatched source length or missing output storage. Coordinates and
counts use checked unsigned arithmetic. Feature extraction crosses packed-word
boundaries through exact source indices; no independent halo emission can
create duplicate anchors. There is no GPU stream or device allocation in this
host branch.

Construction/traffic accounting remains explicit: triplet planes contain four
32-bit words per packed block; model fitting consumes eight targets; bank model
holds four doubles; hard circuit retains eight truth bits plus versions. Output
traffic grows with selected witnesses, and packed decisions do not retain
unselected source detail. The original sequence is retained for later queries;
these witnesses do not establish a lossless embedding or learned genome hierarchy.

Reproduce the dedicated host check from Baseplane with an explicit CE path:

```sh
cmake -S experiments/moonshot -B /tmp/bp-moon-compiler-build \
  -DCMAKE_BUILD_TYPE=Release \
  -DCE_MOON_SOURCE_DIR=/explicit/CE/worktree/experiments/baseplane_moonshot
python /tmp/moonshot_build_slot.py cmake --build /tmp/bp-moon-compiler-build --target bp_moon_compiler_test -j1
ctest --test-dir /tmp/bp-moon-compiler-build -R '^bp_moon_compiler_test$' --output-on-failure -V
```

`receipt.json` records exact commands, source digests and observations. All four
cards have representative host implementations, compiled/run/compared. Native
LOP3 source is emitted by CE; this Baseplane family adds no CUDA kernel and did
not compile or launch a GPU path, run a benchmark or test biological efficacy.
