# C05 — Learning compiles away

This executed host composition connects E29/E30/E31/E19:

CE sequence-derived floating teacher → hardened immediate → typed validity-masked
logic circuit → rewritten Boolean mask → state-conditioned exact predicate bank
→ rank/select descriptor routing → source-window extraction → independent-half
approximate nomination → exact Hamming and motif verification.

The teacher is actually fitted and its returned immediate drives `logic::Circuit`.
Its decisions are compared with floating predictions on source triplets. Rewritten
Boolean output and actual GT-bank hits select routed descriptors; those outputs
admit the source windows used by E19. Reversing context switches to the distinct AC
bank. An invalid-context source produces no learned carrier.

The admitted windows are AAGT, CAGT and CCGT. One half probe misses two pairs within
Hamming distance one. Adding the other half probe nominates all three pairs;
full-string verification retains two and rejects the distance-two pair. Actual
verified pairs then drive exact source replay of both GT witnesses. Pair source
IDs, versions and coordinates remain separate. Near sequence agreement is never
relabelled exact identity. The all-pairs fixture recall calculation inside E19 is
used only to report missed candidates, never to generate them.

Source: `smoke.cpp`; entry points in `families/compiler/compiler.hpp`,
`families/logic/logic.hpp` and `families/rendezvous/rendezvous.hpp`. CE learning owns
teacher fitting, hardening, bank scores and rewriting. The learned question and
rewritten query are distinct predicates composed as filters. Truth-table hardening
loses floating confidence; exact source and witnesses permit revisit. All fixture
buffers are finite (four four-base sources, three nominees). No runtime queue,
trained biological model, throughput or general recall claim is made.

```sh
python /tmp/moonshot_build_slot.py cmake -S experiments/moonshot/compositions/compiled_learning \
  -B /tmp/bp-moon-c05-build -DCE_MOON_SOURCE_DIR=/path/to/cellerator/experiments/baseplane_moonshot
python /tmp/moonshot_build_slot.py cmake --build /tmp/bp-moon-c05-build \
  --target bp_moon_composition_compiled_learning -j1
ctest --test-dir /tmp/bp-moon-c05-build -R '^bp_moon_composition_compiled_learning$' --output-on-failure
```

The child CMake also works under the composition aggregate. Host compiled and
executed. CUDA and benchmarks were not run. Exact provider identity and outcomes
are in `evidence.json`.
