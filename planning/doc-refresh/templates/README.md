# Baseplane

**A genome is not three billion equally expensive tokens. Can its organization guide how we compute on it?**

Baseplane aims to build hierarchical, learned floating-point representations of genome-wide information while staying grounded in exact sequence. Its hypothesis is that useful biological organization can make relevant information computationally close, without forcing every later operation to pay equally for every intervening base.

The useful hierarchy is part of what the encoder must learn. Baseplane therefore cannot assume a complete genome-wide map of biologically meaningful objects before representation begins. It can prepare the machinery; the current information must help decide how that machinery is used.

## Adapt the information flow, not a forest of branches

Nearby sequence is a natural starting point because it is cheap to bring together on a computer, and locality is often a useful biochemical clue. This is an opportunity to exploit—not a compulsory distance weight, an annotation catalogue, or a separate cis/trans architecture. More distant information must remain accessible as the representation develops.

The CUDA experiments ask whether that conditional computation can be expressed through collective operations: turn decisions into masks, regroup or compose information, and allocate richer floating-point work where it is useful. The same question applies at more than one scale. Warp-level bit operations are a concrete experiment in this design, not Baseplane's endpoint.

```text
exact sequence and current representations
    → cheap local/collective computation
    → adaptive composition, routing and refinement
    → richer hierarchical representations
```

The long-term idea extends to other highly parallel accelerators; CUDA is the current experimental platform. Keeping exact sequence available does not make every learned embedding lossless, nor does a routing decision establish a biological mechanism.

## What exists now

{{README_STATUS}}

## What the experiments taught us

{{README_RESULTS}}

The [results index](docs/results/index.md) links to the single selected historical study and the full lab selection report.

## Explore the repository

Start with the [design](docs/design/overview.md), [source map](docs/development/source-map.md), or [build/use guide](docs/development/start.md). The [current snapshot](docs/status/current.md) explains which exact-sequence operations and experimental hierarchy paths exist.

[How the projects fit together](docs/design/program.md): Baseplane owns sequence meaning and provenance; [Cellerator](https://github.com/tumlinso/Cellerator) owns general structured numerical execution; [GlassHelix](https://github.com/tumlinso/GlassHelix) asks which dynamical mechanisms the observations support.

Baseplane is an experimental research project, not yet a validated whole-genome learned model.
