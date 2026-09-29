# CarryFold result: bounded candidate

Deterministic untrained fixture; CUDA 12.9.86, V100 sm_70, Release without fast math. Project Control GPU gate passed at 1025 records; CUDA 12.9 memcheck, initcheck, racecheck and synccheck reported zero errors. On-device current-float coefficient/gate production feeds both the custom and CUB inclusive scans. The same noncommutative left-then-right affine operator is used. The GPU probe also scans four-record coarse summaries and checks source begin/end intervals.

| current 4-float records | custom producer + scan median ms | CUB producer + scan median ms |
|---:|---:|---:|
| 1025 | 0.019456 | 0.014336 |
| 65536 | 0.070656 | 0.059392 |

CUB wins these complete device-resident pipeline measurements. The timer includes coefficient production and all scan/carry launches, with scratch preallocated. It excludes allocation, host fixture construction, H2D/D2H and result interpretation; no end-to-end speed claim follows. The two-level source map is exact, but it does not establish a learned genomic hierarchy. No biological segmentation or trained weights are claimed.

Held-out scalar direct-vs-tree maximum absolute coefficient error over increasing lengths was: n=33: 2.98023e-08, n=1025: 2.98023e-08, n=65536: 5.96046e-08. Two input orders with the same mean coefficients (a=0.5, b=0) produced final b=-0.5 and b=0.5. This witnesses order information lost by averaging.

Source SHA-256: driver dc8f136f7206f2a18be99d975573280034f64a29ce2b6c0148eb3342d69416e5; kernels 1434eb2a4b968c0bde3dbccf7eaeec24312aa51b35c31df1a4c64e907e0df7e6; runner 8f8cac4e76a24abab1b57d846ca044533de70ec17ebe3770f72e2ed758d07a5b.
