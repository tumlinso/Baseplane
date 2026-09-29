# What was checked—and what was not

The C++ scalar references compiled with GNU C++ and passed **1,045,957 assertions** with AddressSanitizer and UndefinedBehaviorSanitizer enabled. These assertions exercise repeated numerical comparisons across fixtures; they are not a million independently designed tests. Fifteen Python tests passed for package/install/result guards. Actual logs and commands are retained here.

Project Control accepted the exact native plan at **Baseplane Todo revision 131**, observed **28 September 2026, 18:39:09 UTC**. It reported seven additions and no dependency, scope or interface conflicts. The plan has **not** been applied. Its two existing-invariant text changes remain explicit for local review; a task-only `would_modify: []` field does not erase those proposed policy changes.

No CUDA compiler or GPU is present in this preparation environment. The CUDA kernels and driver are written, but **uncompiled, unexecuted and unbenchmarked**. The no-toolchain check returns 77, not success. `--all-results` correctly fails while the four actual experimental result files are absent. There are no fabricated device performance records.

The prepared code is deliberately smaller than the intended experiments: several on-device producers, CUB comparisons, and two-level information probes remain precisely specified task outcomes. All numerical weights in supplied fixtures are deterministic **untrained** test parameters. Nothing establishes a learned genome model, lossless embedding or novel general algorithm.
