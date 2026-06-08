# NanoBoyAdvance - Dreamcast Port Roadmap

This is the single source of truth for Dreamcast port planning. Supporting docs
remain focused on facts and procedures:

- `DREAMCAST.md`: user/developer operating guide.
- `DEBUG_REPORT.md`: Kirby crash investigation notes and evidence.
- `COMPATIBILITY.md`: benchmark and compatibility results table.

## Active Phase: Milestone 1 ROM/RAM Stability

**Goal:** reach a deterministic boot, load, play, and exit path on Dreamcast
without full-ROM allocations or undefined behavior after media failures.

### Current Focus

- Kirby & The Amazing Mirror (USA), 16 MiB, is the stress case for large-ROM
  loading and Flycast 32 MB RAM-mod testing.
- Stock Dreamcast remains conservative: ROMs over 8 MiB require either detected
  extended RAM or the explicit Large ROMs setting.
- The current build uses a file-backed paged ROM cache for Dreamcast virtual
  paths (`/cd` and `/pc`) instead of allocating the full cartridge.

### Completed In This Phase

- [x] Replace hardcoded `/cd/rom.gba` flow with ROM browser scanning `/pc/roms`
  and `/cd`.
- [x] Use POSIX directory and file APIs for Dreamcast virtual paths.
- [x] Validate Dreamcast ROMs by reading only the 228-byte GBA header.
- [x] Add `ROMLoader::GetFileSize()` for cheap size preflight.
- [x] Attach Dreamcast ROMs through a paged file backend.
- [x] Detect backup type from file chunks instead of full-ROM memory scans.
- [x] Keep `/pc` saves memory-backed when writable streams are unavailable.
- [x] Add Large ROMs policy with 32 MB RAM auto-detection.
- [x] Preload ROM page 0 before the first CPU instruction fetch.
- [x] Report paged ROM media read failures through a fatal-error screen.
- [x] Handle ISO9660 `;1` filename suffixes with clean-path and raw-path probes.
- [x] Reduce stock-ROM page cache pressure by using two active 1 MiB pages for
  ROMs up to 8 MiB and four pages only for larger ROMs.
- [x] Add page-cache telemetry in the FPS interval log/overlay (`PG` = ROM page
  misses since the previous FPS sample).
- [x] Optimize the paged-ROM read hot path: 16/32-bit fetches now resolve the
  cache page once and read directly from the page buffer instead of fetching
  byte-by-byte (each byte previously re-ran the page lookup and cache scan).
  Boundary-spanning accesses still fall back to the byte-wise path.
- [x] Keep the Dreamcast build green after the latest upstream pull.

### Next Actions

1. Run the Kirby CDI in Flycast with `Dreamcast.RamMod32MB = yes`.
   - Record the last visible breadcrumb if Flycast exits.
   - If it reaches gameplay, enable Show FPS and record FPS plus `PG` page
     misses during any stutter.
2. Build and test one known-good 4 MiB or 8 MiB CDI.
   - If the small ROM fails, focus on KOS file/save APIs.
   - If the small ROM works and Kirby fails, focus on large-ROM cache behavior.
3. Validate failure cases on device/emulator.
   - Missing/invalid BIOS.
   - Missing/invalid ROM.
   - Bad `/cd` media read during gameplay.
   - Exit combo back to ROM browser.
4. If Kirby reaches gameplay but stutters, tune cache policy from telemetry.
   - Tune page size/count only against measured `PG` misses and FPS.

### Acceptance Criteria

- BIOS + ROM load from `/cd` and reach gameplay without a hard crash.
- Large ROMs are blocked on stock memory unless explicitly allowed or 32 MB RAM
  is detected.
- Missing/invalid files and bad media reads show deterministic user-facing
  errors.
- Exit combo returns cleanly to the ROM browser.
- Controller mapping and analog dead-zone behavior remain stable during play.
- Long sessions do not show progressive audio underrun or frame pacing drift.

## Milestone 2: Save + ROM Browser Usability

**Goal:** make the Dreamcast frontend practical for daily play.

- [x] Add ROM selection flow instead of a single hardcoded ROM path.
- [x] Add in-app settings for performance, FPS, large ROMs, frame skip, audio
  buffer, BIOS path, ROM folder, and save folder.
- [x] Store per-ROM saves under `/pc/saves/<rom>.sav`.
- [x] Document VMU as out of scope for now.
- [ ] Re-enable safe config load/save once Flycast `/pc` behavior is fully
  understood.
- [ ] Improve save UX for in-memory-only sessions.
- [ ] Add clearer browser affordances for unavailable large ROMs on stock RAM.

## Milestone 3: Performance + Compatibility

**Goal:** establish repeatable performance expectations and tune against them.

- [x] Add Accuracy / Balanced / Speed profiles.
- [x] Add Show FPS overlay.
- [x] Document benchmark workflow in `DREAMCAST.md`.
- [ ] Lock the benchmark ROM set in `COMPATIBILITY.md`.
- [ ] Capture baseline FPS per profile on retail hardware.
- [ ] Tune CPU/audio/video/cache hot paths against recorded baselines.
- [ ] Fill compatibility tiers, recommended profiles, and known regressions.

## Milestone 4: Release Packaging + Contributor Workflow

**Goal:** make releases and outside contributions repeatable.

- [ ] Standardize build and image creation into one documented release process.
- [ ] Produce release artifact layout and checksum notes.
- [ ] Add contributor docs for Dreamcast-only development and testing.
- [ ] Publish a public roadmap/changelog cadence for future milestones.
