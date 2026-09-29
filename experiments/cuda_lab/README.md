# Baseplane CUDA lab

**Status:** the four bounded synthetic experiments are complete. Read the
[final selection report](results/selection/report.md) for the measured
comparisons, decisions, and claim limits. Its combined-source replay and final
CUDA sanitizer receipts are linked from that report.

The lab evaluated BitLift (Boolean evidence into floating summaries), CarryFold
(ordered affine summaries), Rethread (selected objects into cooperative work),
and Rendezvous (runtime-key peer grouping). CarryFold remains one restricted
research candidate; CUB was the faster measured executor in the reported scan
fixtures. The other three were evaluated and not promoted. All weights and
routing keys were untrained synthetic fixtures. These results do not establish
a trained genome model, biological validity, or organism-scale context
construction.

The lab remains separate from the exact-sequence library. It changed no public
Baseplane API and is not a production numerical interface. For completed run
status and bounded reproduction steps, see [START_HERE.md](START_HERE.md).
