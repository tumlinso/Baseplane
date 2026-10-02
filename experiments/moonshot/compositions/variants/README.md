# C03 — One genome, many variants

This executed host composition connects E33/E34/E35/E44 through real outputs:

`ports::alternative_worlds` → CE delta-world states and exact world sequences →
`incremental::RepeatQuotient` content sharing → `DirtyCone::replace` dependency
IDs → `Memo::reusable` edit guard → reuse or exact recomputation.

The six-base fixture has four worlds: baseline, replacement outside memo support,
replacement inside it, and an invalid-base replacement inside it. Source sequence
from the E44 world producer drives the actual edit descriptors and directory
updates. Returned dirty IDs trigger memo guard inspection. Two worlds reuse the
memo and two recompute it. Every answer is compared against independently
executing the world sequence. The CE numerical world result is separately compared
against a direct sum of valid base loads. Invalid packed payloads contribute to
neither result.

Repeated baseline occurrences share exact content while preserving occurrence
maps. Derived world IDs become adapter source IDs, so distinct alternatives cannot
alias at the same version. Original E44 support retains baseline source ID/version,
and the explicit world ID ties that lineage to each derived source view. This is a
counterfactual identity convention, not a claim of distinct observed genomes.

Source: `smoke.cpp`; entry points in `families/ports/include/bp_moon/ports.hpp` and
`families/incremental/incremental.hpp`. CE owns numerical world evaluation. All
inputs/edits are tiny, vectors bounded by four worlds and six positions, and
query/weight identities remain guarded. Exact sequence is retained for revisit;
GC summaries discard arrangement, numerical world equality does not merge source
identities, and no biological or performance claim is made.

Build with an explicit CE provider (recorded in `evidence.json`):

```sh
python /tmp/moonshot_build_slot.py cmake -S experiments/moonshot/compositions/variants \
  -B /tmp/bp-moon-c03-build -DCE_MOON_SOURCE_DIR=/path/to/cellerator/experiments/baseplane_moonshot
python /tmp/moonshot_build_slot.py cmake --build /tmp/bp-moon-c03-build \
  --target bp_moon_composition_variants -j1
ctest --test-dir /tmp/bp-moon-c03-build -R '^bp_moon_composition_variants$' --output-on-failure
```

The child CMake also works under the composition aggregate. Host compiled and
executed; CUDA, training of genome models, benchmarks and multi-GPU work were not
run.
