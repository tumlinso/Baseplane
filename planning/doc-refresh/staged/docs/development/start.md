# Build, use and validate

Start with one supported path rather than every historical target. Commands below must be qualified against the source version in the [current snapshot](../status/current.md).

```sh
cmake -S . -B build-docs-cpu -DBASEPLANE_ENABLE_CUDA=OFF
cmake --build build-docs-cpu --target baseplaneDna2Test baseplaneBitOpContractTest baseplaneDna2ValidityTest -j 4
./build-docs-cpu/baseplaneDna2Test
./build-docs-cpu/baseplaneBitOpContractTest
./build-docs-cpu/baseplaneDna2ValidityTest
# Isolated lab scalar checks do not require a GPU.
python3 experiments/cuda_lab/tools/run.py --phase host --case all --build-dir build-docs-lab-host
```

For the optional CUDA build use the installed compatible toolchain and `-DBASEPLANE_ENABLE_CUDA=ON`. Build/run the lab with its existing runner and leased GPU procedure; do not invoke a performance phase while merely rebuilding these documents.

## Work on a bounded component

Follow the [source map](source-map.md), inspect its nearest guidance and tests, and keep an example/reference beside the behavior it exercises. A documentation change does not require a full hardware campaign. A source move does require updating build/install/import/include references and rerunning the affected targets.

For measured claims follow [results](../results/index.md). Use assigned resources; record command, source, toolchain, input, precision, outputs, warmup/repeats and timed phases. Keep profiler diagnostics separate from benchmark timings. Preserve the original evidence when rendering new documentation.

## Code and documentation boundaries

Durable design goes in `docs/design/`, practical instructions here, dated summaries in `docs/status/`, and evidence in `docs/results/` linked to original records. Use experiments for unpromoted mechanisms. Generated Todo state and historical notes are not substitute architecture.
