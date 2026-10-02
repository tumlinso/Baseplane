# Final normal-entry host build

Observed 2026-10-02T14:32:12.639343+00:00. Root BP-MOON-150 authorized connecting compositions to the normal isolated moonshot build and updating the dated capability snapshot. The worker changed only the parent experimental CMake entry, the snapshot and this evidence directory. Public exact-library targets and optional CUDA defaults were preserved.

## Exact identities and outcome

Baseplane tested commit: `9f236675da70792db73c4ea945de5c1d96e9341d`, including the hypotheses reverse-coordinate repair `105303ee044ad73cfa297169e725a2a706d97c6f` and normal-entry composition wiring. Canonical Cellerator tested commit: `eb9a131d92cf0dd38de8a6f1833797f6a68214ce`. Source SHA256 identities and the five composition executable SHA256 identities are retained in [identity.json](identity.json).

The GNU C++ 13.3.0 Debug build completed successfully with one compile job through the shared build-slot wrapper. Discovery registered exactly 25 tests and each of C01/C03/C04/C05/C06 once. CTest passed **25/25**, with zero failures and reported total test runtime 3.99 seconds; this is test duration, not performance evidence. The hypotheses test includes exhaustive forward/reverse interval mapping regressions. A separate standalone composition configure also registered 25 tests after parent auto-inclusion, establishing that the recursion guard still works with the new normal entry.

## Commands

Working directory: root-assigned `bp-moon-l-integration` Baseplane managed workspace.

```sh
cmake -S experiments/moonshot -B /tmp/bp-moon-final-normal-debug \
 -DCMAKE_BUILD_TYPE=Debug \
 -DCE_MOON_SOURCE_DIR=/home/tumlinson/Cellerator/experiments/baseplane_moonshot \
 -DBUILD_TESTING=ON
ctest --test-dir /tmp/bp-moon-final-normal-debug --show-only=json-v1
python /tmp/moonshot_build_slot.py cmake --build /tmp/bp-moon-final-normal-debug -j1
ctest --test-dir /tmp/bp-moon-final-normal-debug --output-on-failure \
 --output-junit planning/baseplane_moonshot_bootstrap/results/final-build/ctest.xml
cmake -S experiments/moonshot/compositions -B /tmp/bp-moon-final-standalone-entry \
 -DCMAKE_BUILD_TYPE=Debug \
 -DCE_MOON_SOURCE_DIR=/home/tumlinson/Cellerator/experiments/baseplane_moonshot \
 -DBUILD_TESTING=ON
ctest --test-dir /tmp/bp-moon-final-standalone-entry --show-only=json-v1
```

The actual JUnit command used the absolute evidence file path recorded in identity.json. The normal-entry discovery was checked programmatically for exactly 25 tests and exactly one registration per composition before compiling. All registered host tests ran without a filter. The standalone entry check configured/discovered tests; it did not repeat the executable suite.

## Evidence and limits

[configure.log](configure.log), [build.log](build.log), [test-discovery.json](test-discovery.json), [ctest.log](ctest.log) and [ctest.xml](ctest.xml) preserve the normal-entry evidence. [standalone-configure.log](standalone-configure.log) and [standalone-entry.json](standalone-entry.json) preserve the second entry check. [identity.json](identity.json) records revisions, commands, source/binary hashes and exit codes.

No CUDA compilation, GPU launch, benchmark, sanitizer or biological dataset evaluation was performed by this final host validation task. Prior device and compiler evidence remains separately scoped to its source/receipts. Root retains workflow gates, completion, delivery and any later qualification. The capability snapshot states the bounded isolated-library behavior and explicit remaining compositions without recreating live task status.

Provider HEAD advanced to `fc26f007033763c8d773af536f0d5d015c8f9cf5` during validation. A `git diff --name-only` comparison against the recorded provider revision found only generated Todo snapshot/status/task projections; the consumed provider source did not change. Exact paths are recorded in identity.json. Baseplane HEAD stayed at the tested source revision.
