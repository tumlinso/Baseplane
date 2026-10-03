# Source and sequence question contract

The host API in `Baseplane/seq/source_view.hh` borrows the existing packed exact
sequence and validity sidecar. Physical storage increases in genomic coordinate.
Reverse questions traverse reverse-complement codes while retaining original
global coordinates. Invalid bases remain explicit and their packed payload is
hidden. Ownership is evaluated in the physical chunk's declared halo coordinates.
The owner declares immutable structure epochs and mutable value generations,
keeps the memory alive, and synchronizes changes before calls.

`Baseplane/query/contracts/sequence_question.hh` asks about valid source bases over
a half-open logical support, either owned anchors or the full span including
halos. It rejects mismatched source identity/epoch/generation, malformed spans
and unknown domain/strand tags. Detail emission uses caller-owned host storage;
records follow logical order, reverse global coordinates decrease, and required
capacity includes records that did not fit. Invalid positions and excluded valid
halo anchors have separate counters. No invalid position is emitted as a base.

The four answer types are exact valid counts, certified count bounds, learned
counts with confidence, and detail requests. The structural validator checks
context, numeric ranges and support. The certificate token identifies external
provider evidence; validation does not prove that evidence. Learned confidence
does not become an exact value or certified bound. A detail request can be
fulfilled with a subquestion carrying the same provenance and strand. The exact
count helper computes an actual scalar count from the borrowed source.

This adds no frozen ABI changes, allocation, Cellerator dependency, device adapter,
learned model or biological qualification. The existing sequence core remains
the numerical implementation used by the host tests.

## Native check

```
cmake -S . -B /tmp/bp-is1-seq-native -DBASEPLANE_ENABLE_CUDA=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build /tmp/bp-is1-seq-native --target baseplane_seq baseplaneBitOpContractTest baseplaneDna2ValidityTest -j 2
g++ -std=c++17 -Wall -Wextra -Werror -I include tests/seq/test_sequence_question.cpp /tmp/bp-is1-seq-native/libbaseplane_seq.a -o /tmp/bp-is1-seq-native/sequence_question
/tmp/bp-is1-seq-native/sequence_question
/tmp/bp-is1-seq-native/baseplaneBitOpContractTest
/tmp/bp-is1-seq-native/baseplaneDna2ValidityTest
g++ -std=c++17 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer -I include tests/seq/test_sequence_question.cpp src/seq/dna2_validity.cpp -o /tmp/bp-is1-seq-native/sequence_question_sanitized
/tmp/bp-is1-seq-native/sequence_question_sanitized
```

The test compares packed source calls against a separate text oracle across 11
lengths, both strands, both ownership policies, four capacities, full/interior/empty
supports, invalid and polluted tail payload. It also checks stale provenance,
malformed allocations/halos, coordinate overflow, answer categories and numeric
rejections. Root build registration is returned as a proposed integration change.
