# Final composition aggregate host receipt

Observed 2026-10-02T14:24:38.655564+00:00. Root BP-MOON-140 authorized this bounded final Debug aggregate check after all five composition commits and the C04 empty-object guard fix. This worker made no source edits and owns only the composition inventory update and this validation evidence.

## Source identity and outcome

- Baseplane tested commit: `e5ddae14e6e690b68215fc1ec9586622bf54d6e9` (includes the C04 guard regression fix).
- Cellerator tested commit: `b2a65353d543a8d77d051be2d277d0c6ba4371dc` (reviewed providers plus comparator-only change).
- Configuration: GNU C++ 13.3.0, Debug, BUILD_TESTING=ON, optional CUDA disabled.
- Discovery: exactly 25 host tests registered.
- Build: exit 0, one compile job through the shared build-slot wrapper.
- CTest: **25/25 passed**, zero failed, reported total test runtime 3.56 seconds. This is test duration, not benchmark timing.
- Composition tests: C01 sequence effects, C03 variants, C04 nonlocal context, C05 compiled learning, C06 ports/factors all passed.
- GPU launches, CUDA compilation and timings: none performed by this validation task.

## Executed commands

Working directory was the root-assigned `bp-moon-l-integration` managed Baseplane workspace. The explicit Cellerator source was its independently managed `ce-moon-l-integration` workspace.

```sh
cmake -S experiments/moonshot/compositions -B /tmp/bp-moon-compositions-final-debug \
 -DCMAKE_BUILD_TYPE=Debug \
 -DCE_MOON_SOURCE_DIR=/home/tumlinson/Cellerator/.git/todo-orchestrator/0ccaac37-dbbf-448e-a5f8-def197a70aba/workflow-workspaces/ce-moon-l-integration/experiments/baseplane_moonshot \
 -DBUILD_TESTING=ON
ctest --test-dir /tmp/bp-moon-compositions-final-debug --show-only=json-v1
python /tmp/moonshot_build_slot.py cmake --build /tmp/bp-moon-compositions-final-debug -j1
ctest --test-dir /tmp/bp-moon-compositions-final-debug --output-on-failure \
 --output-junit experiments/moonshot/compositions/validation/ctest.xml
```

The actual JUnit argument was the absolute path to this validation directory, as retained in `identity.json`. The discovery JSON was checked programmatically for exactly 25 tests before building. No tests were filtered or disabled by the final CTest command.

## Evidence

- [identity.json](identity.json): exact commands, tested revisions, observed time, exit codes and five executable SHA256 identities.
- [configure.log](configure.log): successful configure/generate.
- [build.log](build.log): full successful aggregate host build.
- [test-discovery.json](test-discovery.json): all registered tests and their commands.
- [ctest.log](ctest.log): every test result and aggregate summary.
- [ctest.xml](ctest.xml): machine-readable test results and per-test output.

This receipt establishes compilation and the executed bounded host semantic checks. It does not establish trained biological models, whole-genome performance, device execution or every proposed catalogue composition. C02/C07/C08/C09/C10 remain explicitly deferred in the parent inventory. Root retains final workflow gates, task completion, integration and delivery.
