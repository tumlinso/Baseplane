# Installed exact core and optional host components

Base: `21bd3b910add45d357d0b8141197e8f0a3f6b7a7`.

`cmake/substrate/BaseplaneSubstrate.cmake` loads installed optional query,
hierarchy, index, reuse and learning interfaces. Exact `Baseplane::seq` remains
independent: loading its package alone never looks up Cellerator. Higher components
require actual installed headers and imported selected CE targets. The agreed
package/namespace is `Cellerator` / `Cellerator::`; the explicit package name can
be overridden by a caller. Provider targets retain their existing names:
`moonshot_effects`, `moonshot_mechanisms`, `moonshot_learning`.
No sibling source paths, copied leaf source or fabricated CE capability is used.

Optional interfaces expose host availability and explicitly mark substrate CUDA
execution unavailable. Required CUDA rejects these host APIs. Recursive target
checks detect cycles before returning a selected component. A missing imported
provider rejects with its exact capability name.

`cmake/substrate/root-integration.patch` is the proposed shared build/config edit
for MERGE-A: install/load this module and resolve requested package components.
It applies cleanly to this baseline. It has not been applied here. Root owns
acceptance and integrated worktree lineage.

## Qualified baseline gate

```
python3 tests/substrate/install/run_host.py
```

The driver configures/builds/installs the actual exact core with CUDA/Highway off,
relocates the package, then compiles/runs an independent installed consumer.
The consumer calls native validity/window functions through `Baseplane::seq`.
The actual imported exact-core target graph is checked. Missing selected installed
CE capability, required unintegrated substrate and CPU-only CUDA qualification
reject explicitly. No fake CE target is created in any fixture.

Owned gate inputs:
`cmake/substrate/BaseplaneSubstrate.cmake`,
`examples/substrate/install/CMakeLists.txt`,
`examples/substrate/install/consumer.cpp`,
`tests/substrate/install/missing-provider/CMakeLists.txt`,
`tests/substrate/install/run_host.py`.
The driver reports installed package/header/library hashes. The native root
CMake/core are baseline dependencies, inspected but not modified by this leaf.

## Required integrated consumer after MERGE-A

```
python3 tests/substrate/install/run_host.py --require-integrated --baseplane-prefix /actual/integrated/baseplane/install --ce-prefix /actual/integrated/cellerator/install
```

This mode requires actual installed Baseplane and CE prefixes. It fails if the
module, any integrated header or any selected imported CE capability is missing.
The compiled consumer calls query exact-count, CE hierarchy composition, global
postings and CE three-role factors, seam edit invalidation, and CE-fitted learned
versus constructed routes through installed targets. It also checks the explicit
host-only CUDA rejection. The input hashes include integrated headers and CE
package config/exports.

This combined fixture is prepared for the real initial CE SDK and BP MERGE-A.
It has not run here: the frozen baseline contains no integrated leaf headers,
and the selected CE exports await CE BUILD/MERGE-A. No future installed provider
pass, GPU/runtime qualification, biological result or performance claim is recorded.
