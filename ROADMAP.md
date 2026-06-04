# NanoBoyAdvance - Dreamcast Port Roadmap

This roadmap outlines the planned milestones for completing the Dreamcast port of NanoBoyAdvance.

## Milestone 1: Stable Boot/Play Baseline
**Goal:** Stabilize the core runtime on Dreamcast and lock down a crash-free experience for a single game.
- [x] Define Dreamcast-only project scope (docs updated)
- [ ] Lock down crash-free boot/load/exit flow for BIOS + ROM from `/cd`
- [ ] Harden error handling for missing/invalid BIOS/ROM and bad media reads
- [ ] Validate controller input mapping consistency and dead-zone behavior
- [ ] Confirm audio/video timing is stable across long play sessions

## Milestone 2: Save + ROM Browser Usability
**Goal:** Close critical frontend gaps to make the emulator usable for daily play.
- [ ] Implement save pipeline appropriate for Dreamcast workflows (VMU/filesystem policy and UX)
- [ ] Add ROM selection flow (menu/browser) instead of hardcoded single ROM path
- [ ] Add in-app settings surface for key options (frame skip, audio latency, BIOS/ROM path behavior)
- [ ] Improve user-facing overlays/messages for loading, errors, and exit combo

## Milestone 3: Performance Profile and Compatibility Pass
**Goal:** Maximize frame rates and establish clear compatibility expectations.
- [ ] Establish a repeatable Dreamcast benchmark/test ROM set
- [ ] Tune CPU/audio/video hot paths and buffering for full-speed target where possible
- [ ] Add optional performance profiles (accuracy-first vs speed-first)
- [ ] Document per-game compatibility tiers and known regressions

## Milestone 4: Release Packaging + Contributor Workflow
**Goal:** Finalize the project for public releases and outside contributions.
- [ ] Standardize build + image creation into one documented release process
- [ ] Produce release artifacts and disc layout expectations with checksum notes
- [ ] Add contributor docs specifically for Dreamcast-only development and testing
- [ ] Publish a public roadmap/changelog cadence for future milestones
