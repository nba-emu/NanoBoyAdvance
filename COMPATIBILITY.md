# NanoBoyAdvance – Dreamcast Compatibility & Benchmarks

This document tracks per-game compatibility tiers, recommended performance
profiles, and known regressions for the Dreamcast port. It is the canonical
record for the Milestone 3 benchmark/compatibility pass.

See [`DREAMCAST.md`](DREAMCAST.md#repeatable-benchmark-workflow) for the
benchmark workflow and the tier definitions.

## Tier Definitions

| Tier | Meaning |
|------|---------|
| **Playable** | Holds ~full speed (~59.7 FPS) on at least one profile, no game-breaking issues. |
| **Runs** | Boots and is playable but with noticeable slowdown or audio/video artifacts. |
| **Broken** | Fails to boot, hangs, or has issues that prevent normal play. |
| **Untested** | Not yet validated on hardware. |

## Benchmark ROM Set

The benchmark set is chosen to span the GBA workload range so results
generalize. Fill in the concrete ROMs used so runs stay reproducible. Use
homebrew or legally owned dumps only.

| Slot | Workload profile | ROM used | Benchmark scene |
|------|------------------|----------|-----------------|
| 1 | Light 2D / menu-driven | _TBD_ | _TBD_ |
| 2 | Sprite-heavy action | _TBD_ | _TBD_ |
| 3 | Mode-7 / pseudo-3D | _TBD_ | _TBD_ |
| 4 | Audio-heavy (HLE-sensitive) | _TBD_ | _TBD_ |

## Benchmark Results

Steady-state FPS per profile for each benchmark scene. Record on retail
Dreamcast hardware following the workflow in `DREAMCAST.md`.

| ROM | Scene | Accuracy | Balanced | Speed | Recommended profile |
|-----|-------|----------|----------|-------|---------------------|
| _TBD_ | _TBD_ | _–_ | _–_ | _–_ | _–_ |

## Compatibility Table

| Game | Tier | Recommended profile | Notes / known regressions |
|------|------|---------------------|---------------------------|
| _No titles validated on hardware yet._ | Untested | – | – |

## Known Regressions

Track Dreamcast-specific regressions here as they are found (audio crackle on
specific mixers, video tearing in certain modes, save corruption edge cases,
etc.). Each entry should note the affected games, the profile(s) involved, and
a reproduction.

- _None recorded yet._
