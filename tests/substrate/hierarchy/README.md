# Usable host hierarchy component

Base: `21bd3b910add45d357d0b8141197e8f0a3f6b7a7`.

`Baseplane/representation/hierarchy.hh` provides an immutable owned exact-source
snapshot shared by summary trees, residual carriers and exact detail revisits.
It copies validated packed input and validity, retains chunk identity, structure
epoch and value generation, and checks those tokens on queries/revisits.
The native `seq::get_base`, validity and coordinate functions remain the exact core.
Reverse orientation traverses reverse-complement bases with original coordinates.

Callers supply five sequence-specific two-channel affine effects (four canonical
bases and an explicit invalid transition), plus any number of competing boundary
sets. Each set forms an ordered summary hierarchy over nonuniform spans. Histogram
and affine summaries coexist in every node. Whole covered nodes answer without
source replay; partial leaf supports revisit the exact snapshot, with replay
counts returned. Construction counts include every leaf scan for every alternative
and the carrier scan; chosen-boundary scoring happens before construction and is
an additional source pass. Halo bases participate in region summaries; detail
revisits exclude unowned anchors by default and can explicitly include halos.

`choose_cuts` builds input-dependent boundaries with caller-owned fitted or
chosen scoring parameters, minimum/maximum lengths and an explicit threshold.
It makes no learned quality claim. Final tails can be shorter than the minimum.
Lifting uses signed GC/AT carriers only on valid source support. CE `lift/unlift`
provide coarse/detail values; omitted details are explicitly lossy. An odd valid
tail remains exact. Carrier positions preserve the mapping back to source.

Generic affine composition/application and reversible lifting are exclusively
the existing Cellerator `ce_moon::MonomialAffine`, `compose/apply` and `lift/unlift`
provider in `experiments/baseplane_moonshot/include/ce_moon/reference.hpp`.
The native consumer selects `Cellerator::moonshot_effects`; these are actual
experimental host providers. Installed numerical component exports, CUDA
execution, arbitrary learned latent reconstruction, certified jet bounds, fitted
boundary training and biological/performance qualification remain unsupported.
No substitute arithmetic, fabricated certificate or benchmark is introduced.
Future certified approximations must come from their CE numerical owner before
being labeled certified. The shared ownership API stays independent of new SEQ
leaf headers and requires no cross-leaf unpublished source.

## Focused native gate

```
cmake -S tests/substrate/hierarchy -B /tmp/bp-is1-hierarchy-native -DCE_MOON_SOURCE_DIR=/home/tumlinson/Cellerator/experiments/baseplane_moonshot -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/bp-is1-hierarchy-native --target baseplaneHierarchyTest -j 2
ctest --test-dir /tmp/bp-is1-hierarchy-native -R '^baseplaneHierarchyTest$' --output-on-failure
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -I include -isystem /home/tumlinson/Cellerator/experiments/baseplane_moonshot/include tests/substrate/hierarchy/test_hierarchy.cpp src/seq/dna2_validity.cpp -o /tmp/bp-is1-hierarchy-native/hierarchy_sanitized
/tmp/bp-is1-hierarchy-native/hierarchy_sanitized
```

Required inputs: this header and test/CMake fixture; existing
`cmake/BaseplaneRepresentation.cmake`, root sequence CMake/source/headers;
the actual CE provider directory passed above (its CMake entry and headers).
CTest checks independent text/count/numerical scalar references, invalid source
payload, odd/empty/tail support, forward/reverse coordinates, halo ownership,
capacity accounting, alternate boundaries/replay costs, coarse/detail recovery,
ordering and stale/unsupported input rejections.

Shared root build proposal: expose an opt-in representation interface using
`baseplane_add_representation_target(... COMPONENTS Cellerator::moonshot_effects)`
after loading the explicit provider, and register this consumer. BUILD owns
installed provider/export qualification. No root CMake edit is included here.
