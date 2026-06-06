# Dreamcast Kirby Crash Debug Report

## Test Case

- ROM: `C:\Users\allen\Downloads\Kirby & the Amazing Mirror\Kirby & The Amazing Mirror (USA).gba`
- ROM size: 16,777,216 bytes / 16 MiB
- Test image: `C:\Users\allen\NanoBoyAdvance-DC\build-dc\disc-test\nba-kirby.cdi`
- Emulator: Flycast `C:\Users\allen\Downloads\flycast-win64-2.6\flycast.exe`
- Flycast setting changed for testing: `Dreamcast.RamMod32MB = yes`

## Observed Behavior

- Booting the CDI reaches the NanoBoyAdvance Dreamcast ROM menu.
- The menu originally showed several entries for the same ROM because the `/cd`
  fallback inserted multiple guessed filenames.
- Selecting the ROM still crashes/exits Flycast instead of entering gameplay or
  showing a stable fatal-error screen.

## Current Local Changes

- `src/platform/dreamcast/src/dc_rom_browser.cc`
  - Uses POSIX `opendir`/`readdir` for all directories including `/cd`.
    On real KallistiOS hardware this correctly enumerates the disc root.
  - Removed the hardcoded `Kirby.gba` candidate list; any `.gba` / `.GBA`
    file present in `/cd` or `/pc/roms` is now discovered automatically.
  - ISO9660 version suffixes (e.g. `GAME.GBA;1`) are stripped from display
    labels but preserved in the file path for `fopen`.
  - `last_rom` existence check uses `fopen` instead of `fs::exists` for
    `/cd` and `/pc` paths, which may not work through the KOS VFS layer.
  - Validation of ROM entries now only reads the 228-byte header (see below).

- `src/platform/core/src/loader/rom.cc`
  - Adds a Dreamcast-specific `fopen`/`fread` path for `/cd` and `/pc`.
  - Adds `ROMLoader::GetFileSize()` so the Dreamcast launcher can check ROM
    size without allocating and reading the whole cartridge.
  - Dreamcast virtual paths now attach a file-backed paged ROM instead of
    allocating the full cartridge into `std::vector<u8>`.
  - `ROMLoader::Validate()` for Dreamcast virtual paths now reads only the
    228-byte GBA header instead of the full ROM, preventing a 16 MiB
    allocation just to populate the ROM browser.
  - `GetBackupTypeFromFile()` scans the ROM file in 64 KiB chunks for save
    backup signatures (`SRAM_V`, `EEPROM_V`, `FLASH_V`, etc.).  This enables
    correct backup type detection for ROMs not in the game database, without
    loading the entire file into memory.

- `src/nba/include/nba/rom/rom.hh`
  - Adds public `IsPagedROM()`, `GetPagedROMSize()`, and `GetPagedROMPath()`
    accessors so the core layer can inspect paged ROM state.

- `src/nba/src/core.cc`
  - `SearchSoundMainRAM()` now falls through to `SearchSoundMainRAMFromFile()`
    when the ROM vector is empty (paged ROM case on Dreamcast).
  - `SearchSoundMainRAMFromFile()` reads the ROM file in 64 KiB chunks and
    performs the same CRC32 scan, enabling MP2K HLE on the Speed profile
    even for large paged-ROM cartridges.

- `src/nba/include/nba/rom/rom.hh`
  - Adds a Dreamcast-only 4 MiB page cache for cartridge reads, split into
    four 1 MiB LRU pages.
  - Keeps the existing `std::vector<u8>` ROM backend for all non-Dreamcast
    builds.

- `src/platform/dreamcast/src/dc_frontend.cc`
  - Adds vblank pacing to menu loops.
  - Removes config save on ROM selection, avoiding a `/pc/nba-dc.toml` write
    before launching.

- `src/platform/dreamcast/src/device/dc_video_device.cc`
  - Refreshes `vram_s` before direct framebuffer writes.

- `src/platform/dreamcast/src/main.cc`
  - Uses `video_device->Present()` in the emulation loop.
  - Prints and displays launch breadcrumbs before BIOS validation, ROM precheck,
    save path selection, audio setup, core creation, BIOS load, ROM load, and
    reset.
  - Performs a cheap ROM-size preflight before creating the audio/core stack.
  - Skips the duplicate full-ROM `ROMLoader::Validate()` read for Dreamcast
    virtual paths (`/cd` and `/pc`). This means Kirby is no longer read once
    during validation and a second time during load.
  - Adds first-frame breadcrumbs before and after the first three CPU frames.

- `src/nba/include/nba/rom/backup/backup_file.hh`
  - Makes `/pc/...` save files memory-backed on Dreamcast, avoiding
    `std::filesystem` and `std::fstream` during launch.

- `src/platform/core/src/game_db.cc`
  - Adds Kirby & The Amazing Mirror (USA), game code `B8KE`, as SRAM-backed.
    The local ROM also contains an `SRAM_V` signature.

## Leading Crash Hypotheses

1. Memory pressure from full-ROM loading.
   - Kirby is 16 MiB.
   - Previous builds stored the full ROM in `std::vector<u8>`.
   - Current diagnostic build uses a 4 MiB page cache for Dreamcast virtual
     paths, so the 16 MiB cartridge allocation should no longer occur.
   - GBA core state, audio buffers, PPU output buffers, save backup objects, and
     KOS/Flycast runtime allocations compete with the same Dreamcast memory
     budget.

2. Backup save creation on `/pc/saves`.
   - `GetSavePath()` maps `/cd/Kirby.gba` to `/pc/saves/Kirby.sav`.
   - Current diagnostic build intercepts `/pc/...` in
     `BackupFile::OpenOrCreate()` and creates an in-memory buffer instead.
   - If the crash persists past "Phase 7: ROM load", save-file I/O is less
     likely to be the primary cause.

3. `/cd` ISO9660 filename handling.
   - KOS may expose ISO names with version suffixes or uppercase names depending
     on mkisofs/CDI layout.
   - The current CDI places `Kirby.gba` at the disc root.
   - The browser is forced to that exact name to avoid `/cd` enumeration crash.

4. Duplicate ROM reads during launch.
   - Previous builds validated `/cd/Kirby.gba` by reading the whole 16 MiB ROM,
     then read it again during `ROMLoader::Load()`.
   - Current diagnostic build skips that pre-validation on Dreamcast virtual
     paths. A crash at "Phase 7: ROM load" now points at the real ROM
     attach path, backup creation, or first page open/read.

## MIT-Licensed Open-Source Candidates

- SkyEmu: MIT licensed, C/C++-family low-level handheld emulator with GBA support.
  Most relevant for memory layout, save handling, and lightweight platform
  integration ideas.
  URL: https://github.com/skylersaleh/SkyEmu

  Findings applied to this diagnostic branch:
  - SkyEmu keeps GBA cartridge state simple: `rom_size`, a `cart_rom` pointer,
    and a fixed 128 KiB in-memory backup buffer.
  - SkyEmu performs an explicit GBA ROM size check before binding the ROM to
    the core.
  - SkyEmu still expects ROM bytes to be memory-resident for hot-path cartridge
    reads, so it does not provide a drop-in paged ROM backend.
  - The useful portable idea here is the launch contract: size-check first,
    keep saves memory-backed, and avoid duplicate ROM reads. The page-cache
    implementation is local to this port.

- RustBoyAdvance-NG: MIT licensed GBA emulator/debugger in Rust.
  Useful for architectural comparison, especially save types and ROM ownership,
  but less directly copyable into this C++ codebase.
  URL: https://github.com/michelhe/rustboyadvance-ng

- Magia: MIT licensed GBA emulator in Go, WIP.
  Useful as a reference but less mature and less directly portable.
  URL: https://github.com/pokemium/magia

## Recommended Next Fixes

1. Confirm the breadcrumb phase where Flycast exits.
   - Crash before "Phase 7: ROM load" means launch setup still has a platform
     issue.
   - Crash during or immediately after "Phase 7: ROM load" means memory pressure
     or ROM attach is the leading cause.

2. Measure the paged reader's runtime behavior.
   - The current page cache has four 1 MiB pages.
   - If gameplay starts but stutters badly, tune page size/cache count.
   - If gameplay crashes later, inspect DMA/MP2K paths that expect a host ROM
     pointer.

3. Add a ROM-size policy before allocation.
   - For short-term stability, reject >8 MiB on stock Dreamcast and show a clear
     message.
   - For 32 MB mod testing, allow large ROMs only after confirming remaining
     heap.

4. Build a small known-good 4 MiB or 8 MiB GBA ROM CDI.
   - If small ROMs launch, the Kirby failure is almost certainly large-ROM memory
     handling.
   - If small ROMs also crash, focus on `/pc` save creation or KOS file APIs.
