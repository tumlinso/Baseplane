# Focused composition gate

`check.py --ce-sdk PREFIX --bp-sdk PREFIX --ce-core-commit REVISION` configures an
external CMake consumer using the actual installed BP/CE imported targets,
checks the CE config's built source revision, builds and runs the C++ consumer.
Add `--build-dir DIRECTORY` to choose the retained build/evidence location.
The default creates a retained temporary directory and prints exact evidence/log
paths. Failures return nonzero and remain in `check.log`.

The consumer compares effects with an independent ordered text oracle; verifies
AC/CA order sensitivity, exact detail and GC results, zero/truncated output
capacity with sentinels, repeated nonlocal pairs with source support, a real
fitted triplet model, soft/constructed/hardened conventions, explicit lossy
carriers, stale/wrong source rejection, invalid/empty inputs and halos, equal-size
edit repair against full rebuild, and resize epoch changes. No GPU or timing
campaign runs. Native authority/accepted-source prerequisites are separately
checked by the root's `verify_native_core.py` gate; SDK config identity alone
is not an authority receipt.
