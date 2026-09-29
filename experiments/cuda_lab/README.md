# Baseplane CUDA lab

**Start:** [START_HERE.md](START_HERE.md) · [research and shortlist](RESEARCH.md) · [native epic](planning/epic.v2.json).

Four unpromoted mechanisms: BitLift (Boolean evidence → floats), CarryFold (conditional history → associative summaries), Rethread (selected records → cooperative warps), Rendezvous (runtime keys → peer masks).

One CMake project, no extra runtime or model download. Host/scalar checks work without CUDA. The GPU starting implementation targets CUDA12.9/Volta and is explicitly uncompiled/unrun until the local agent executes the CUDA gates. This package is additive and does not modify the live Baseplane workspace or Todo.

Source files are experimental specifications and scaffolds, not published biological results or claimed performance gains. See DOCTRINE.md for scope, experiment briefs for missing work, and evidence/ for actual preparation receipts.
