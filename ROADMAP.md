# NanoBoyAdvance - Dreamcast Port Roadmap

This roadmap outlines the planned milestones for completing the Dreamcast port of NanoBoyAdvance.

## Milestone 1: Stable Boot/Play Baseline
**Goal:** Stabilize the core runtime on Dreamcast and lock down a crash-free experience for a single game.
- [x] Define Dreamcast-only project scope (docs updated)
- [ ] Lock down crash-free boot/load/exit flow for BIOS + ROM from `/cd`
- [ ] Harden error handling for missing/invalid BIOS/ROM and bad media reads
- [ ] Validate controller input mapping consistency and dead-zone behavior
- [ ] Confirm audio/video timing is stable across long play sessions

### Milestone 1 Current Baseline (2026-06-05)
- Boot path is fixed to `/cd/bios.bin` and `/cd/rom.gba`, then enters a single-threaded frame loop paced to ~59.73 Hz.
- Save data is written to `/pc/rom.sav` on writable media instead of read-only `/cd`.
- Exit path is Start + A + B + X + Y held for ~1 second (60 frames).
- BIOS and ROM are validated before core/audio init; loader failures show on-screen text plus serial output, then wait for Start.
- Loader read paths verify full file reads and ROM header validity (`fixed_96h == 0x96`).
- Controller disconnect clears key state; opposing directional inputs are resolved to prevent stuck movement.
- Desktop/CI-side Dreamcast build validation currently depends on KallistiOS tooling; local baseline build failed without `KOS_BASE` and Ninja.

### Reproducible Milestone 1 Failure Cases
- Missing BIOS file: remove or rename `/cd/bios.bin`; expected current result is startup failure before ROM load.
- Missing ROM file: remove or rename `/cd/rom.gba`; expected current result is startup failure after BIOS load.
- Invalid BIOS data: place a non-BIOS file at `/cd/bios.bin`; expected current result is BIOS loader failure.
- Invalid ROM data: place a non-ROM file at `/cd/rom.gba`; expected current result is ROM loader failure.
- Bad media read path: simulate unreadable `/cd` content (disc/ODE access issue); expected current result is loader failure and process exit.

### Milestone 1 Execution Order (Required)
1. Harden BIOS/ROM/media-read error handling first.
2. Validate controller mapping consistency and analog dead-zone behavior second.
3. Validate long-session audio/video timing stability third.

### Milestone 1 Acceptance Criteria
- **Crash-free boot/load/exit flow**
  - BIOS + ROM load from `/cd` without crash and reach active gameplay loop.
  - Exit combo returns cleanly to loader/shell without freeze or watchdog reset.
  - Validate on retail Dreamcast hardware before checking off.
- **Error handling hardening**
  - Missing/invalid BIOS and ROM cases show deterministic user-facing failure behavior.
  - Bad media read conditions fail gracefully without undefined behavior or hard lock.
  - Validate all listed failure cases on-device before checking off.
- **Input mapping and dead zone**
  - Digital button mappings match documented Dreamcast→GBA table in normal play.
  - Analog dead-zone behavior is consistent and does not create stuck directional input.
  - Validate with repeated directional transitions and mixed analog/digital use on-device.
- **Long-session timing stability**
  - No progressive audio underrun artifacts during extended play sessions.
  - Frame pacing remains stable with no persistent drift symptoms after long runs.
  - Validate via multi-session on-device play checks before checking off.

### Milestone Transition Gate
- Milestone 1 checkboxes remain unchecked until acceptance criteria are validated on-device.
- Milestone 2 implementation starts immediately after all Milestone 1 items are validated and checked.

## Milestone 2: Save + ROM Browser Usability
**Goal:** Close critical frontend gaps to make the emulator usable for daily play.
- [ ] Implement save pipeline appropriate for Dreamcast workflows (VMU/filesystem policy and UX)
- [ ] Add ROM selection flow (menu/browser) instead of hardcoded single ROM path
- [ ] Add in-app settings surface for key options (frame skip, audio latency, BIOS/ROM path behavior)
- [ ] Improve user-facing overlays/messages for loading, errors, and exit combo

### Milestone 2 Current Baseline (2026-06-05)
- ROM browser scans `/pc/roms` and `/cd` for valid `.gba` files.
- Per-ROM save files are stored under `/pc/saves/<rom>.sav` (configurable).
- Settings persist to `/pc/nba-dc.toml` (frame skip, audio buffer, BIOS/ROM/save paths).
- In-app menus use on-screen text with A/B/Y/Start navigation.
- Loading and exit overlays show current paths and exit combo hints.
- VMU saves remain out of scope; filesystem policy is documented in `DREAMCAST.md`.

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
