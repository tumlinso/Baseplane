# BP-MOON-050 implementation receipt

Observed 2 October 2026. All four cards are implemented experimentally in a CPU executable. The root owns workflow completion and integration; this worker authored only `experiments/moonshot/families/hierarchy` under its assigned managed workspace. No GPU launches, benchmark timing or Todo mutation was performed by this worker.

Authoritative preflight: native `project_control.inspect(project=baseplane, kind=task, target=BP-MOON-050)` observed revision 220 with prerequisite BP-MOON-010 active. Implementation started only after the root reported its BP050 claim and assigned managed workspace `bp-moon-l-hierarchy`; bounded source handle `wfc_3qCdQaZCi1nn0u4GMcdai-kw-n17jf9127IGVgp_jvU` was supplied by the root.

## Exact executed commands

Working directory:
`/home/tumlinson/Baseplane/.git/todo-orchestrator/1a7de0f4-347d-4707-9fc3-5f89d7c4d6b3/workflow-workspaces/bp-moon-l-hierarchy`

```sh
cmake -S experiments/moonshot -B /tmp/bp-moon-hierarchy-build \
 -DCE_MOON_SOURCE_DIR=/home/tumlinson/Cellerator/experiments/baseplane_moonshot \
 -DCE_MOON_LEARNING_DIR=/home/tumlinson/Cellerator/.git/todo-orchestrator/0ccaac37-dbbf-448e-a5f8-def197a70aba/workflow-workspaces/ce-moon-l-learning/experiments/baseplane_moonshot/families/learning \
 -DBUILD_TESTING=ON
python /tmp/moonshot_build_slot.py cmake --build /tmp/bp-moon-hierarchy-build --target bp_moon_hierarchy_smoke -j1
ctest --test-dir /tmp/bp-moon-hierarchy-build -R '^bp_moon_hierarchy_smoke$' --output-on-failure
/tmp/bp-moon-hierarchy-build/families/hierarchy/bp_moon_hierarchy_smoke > experiments/moonshot/families/hierarchy/semantic-output.txt
git diff --check -- experiments/moonshot/families/hierarchy
```

GNU C++ 13.3.0 compiled the target. The focused CTest passed 1/1 in 0.04 seconds; that duration is test runtime, not a benchmark. `semantic-output.txt` contains actual probe output. The build used the existing shared Baseplane fixture and Cellerator numerical provider, with no hardcoded sibling path in source/CMake.

Cellerator foundation HEAD observed `544a1ae6e2925146f78a6ef62123ba5257c45251`; numerical `include/ce_moon/reference.hpp` SHA256 `dcb8b316a823c48fc7ffcbbee0cd7ab7d9bf06e3935d5fcdf972a751269138ed`. The CE040 learning managed workspace had the same HEAD at test time and its new `families/learning/learning.hpp` SHA256 was `a4fa4c3312fb26492a6cb217b4aad91aa17be95a4e43ff7fe442a9727d4ba8f3`; that header was still worker-authored uncommitted provider input. Root must integrate the actual provider commit before final cross-repository acceptance. The family consumes `Cellerator::moonshot_learning` automatically when available; the explicit learning-directory flag supports the concurrent development wave.

## Checks and limits

E13 tests alternating valid GC/AT carriers, invalid exclusion, retained-detail roundtrip, coarse omission changing a query, version-checked replay, an odd valid tail, reverse coordinates and overflow rejection. Coarse and residual buffers are separate. Exact emitter capacity two records produces twelve, stores two and drops ten.

E14 fits only the Cellerator scorer. Sequence-derived synthetic targets are present only during training. Inference constructs cuts from sequence-local features and learned weights under length caps. Tests compare learned/fixed/surprise cuts and exact chunk reuse after insertion, reconstruct every original base, validate inverse maps including invalid positions, and check reverse-coordinate recovery. The fitted scorer is a synthetic segmentation demonstration, not a biological hierarchy.

E15 verifies conservative threshold intervals, strict equality, masked invalid bases, explicit span limits, and additional precision only for ambiguous comparisons. Its integer-valued sequence latent has finite exact precision; arbitrary floating-point embeddings remain outside this certification.

E16 verifies content seams, exact-byte reuse after insertion, maximum length under repetitive input, minimum length except the terminal tail, a motif crossing primary seams, replay when both forests cut, invalid sequence rejection and invalid cap rejection. The scalar query scans candidate windows exhaustively and has no optimized routing claim.

An initial E13 test used paired homogeneous GC/AT carriers and correctly failed the required omission-change assertion; the fixture was changed to alternating G/A to test the intended loss. No implementation failure was hidden. No CUDA compile/device path, compressed reservoir, multi-level learned model, production buffer ABI, organism dataset or throughput comparison was run.
