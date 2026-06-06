# NanoBoyAdvance – Dreamcast Port

This document describes how to build and run NanoBoyAdvance on the Sega Dreamcast using [DreamSDK](https://www.dreamsdk.org/) and [KallistiOS](http://gamedev.allusions.net/softprj/kos/).

## Prerequisites

- **DreamSDK** (Windows) or a manual KallistiOS toolchain (Linux/macOS)
- **sh-elf GCC** toolchain with C++20 support (GCC 10+ recommended)
- **CMake** 3.24 or newer
- **Ninja** (recommended generator)

## Setting Up the Environment

### DreamSDK (Windows)

1. Install DreamSDK from <https://www.dreamsdk.org/>
2. Open the DreamSDK Shell
3. The environment variables (`KOS_BASE`, `KOS_CC_BASE`, etc.) are set automatically

### Manual KallistiOS (Linux/macOS)

1. Build and install KallistiOS per its [documentation](http://gamedev.allusions.net/softprj/kos/)
2. Source the environment script:
   ```sh
   source /opt/toolchains/dc/kos/environ.sh
   ```

## Building

From the repository root:

```sh
cmake -B build-dc -G Ninja \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-dreamcast.cmake \
  -DPLATFORM_DREAMCAST=ON \
  -DCMAKE_BUILD_TYPE=Release

cmake --build build-dc
```

The output binary is `build-dc/bin/dreamcast/Release/NanoBoyAdvance.elf`.

### Creating a Bootable CD Image

Use KallistiOS `makeip` and `mkisofs` tools:

```sh
# Create the IP.BIN bootstrap
makeip ip.txt IP.BIN

# Scramble the ELF to 1ST_READ.BIN
sh-elf-objcopy -O binary build-dc/bin/dreamcast/Release/NanoBoyAdvance.elf NanoBoyAdvance.bin
/opt/toolchains/dc/kos/utils/scramble/scramble NanoBoyAdvance.bin 1ST_READ.BIN

# Create disc layout directory
mkdir -p cd_root
cp 1ST_READ.BIN cd_root/
cp your_bios.bin cd_root/bios.bin

# Optional: place ROMs on disc root or use /pc/roms on writable media
mkdir -p pc_root/roms pc_root/saves
cp your_rom.gba pc_root/roms/

# Create the ISO (adjust layout for your loader/ODE workflow)
mkisofs -C 0,11702 -V "NBA_DC" -G IP.BIN -l -o nba-dc.iso cd_root/
```

## File Layout

| File / Folder | Path | Description |
|---------------|------|-------------|
| GBA BIOS | `/cd/bios.bin` or `/pc/bios.bin` | Required. Configurable in settings |
| GBA ROMs | `/pc/roms/*.gba` or `/cd/*.gba` | Scanned by the in-app ROM browser |
| Save data | `/pc/saves/<rom>.sav` | Per-ROM backup saves (configurable folder) |
| Settings | `/pc/nba-dc.toml` | Frame skip, audio buffer, path overrides |

Writable `/pc` paths require an SD/IDE adapter or equivalent host filesystem mount.

### Save Policy

- Backup saves are written to the writable filesystem (`/pc/saves` by default).
- Each ROM gets its own `<rom-stem>.sav` file.
- The save directory is created automatically with `mkdir` on first use.
- On real KallistiOS hardware the save file is opened and written through the
  KOS virtual filesystem; on Flycast (where the `/pc/` stream may not support
  writes) saves remain in-memory for the session only.
- Existing save files from previous sessions are loaded on startup even on
  Flycast if the file can be read via `fopen`.
- **VMU saves are not supported yet** — VMU remains planned for a future milestone.

### ROM Loading

- Large ROMs are loaded using a 4 MiB paged file cache (four 1 MiB pages with
  LRU eviction) rather than allocating the full cartridge into main RAM.
- Page 0 is preloaded during ROM attach so the first CPU instruction fetch
  does not block on CD I/O.
- ROM validation reads only the 228-byte GBA header; the full file is never
  read into memory during browser scanning or pre-launch validation.
- Backup type is detected by scanning the ROM file in 64 KiB chunks for save
  signature strings when the game is not in the built-in database.
- ROMs over 8 MiB are blocked on stock 16 MB hardware unless **Large ROMs
  (>8 MiB)** is enabled in Settings.  On a 32 MB RAM mod (or Flycast with
  `RamMod32MB`), KallistiOS reports extended RAM via `DBL_MEM` and large ROMs
  are allowed automatically.
- The ROM browser shows each cartridge size and marks titles that need the
  Large ROMs setting with `[Large ROMs]`.

### Flycast Testing Notes

- The ROM browser enumerates `/cd` using POSIX `opendir`/`readdir`.  On real
  KallistiOS hardware this works correctly.  On Flycast, `opendir("/cd")` may
  behave differently; if the disc root appears empty, copy your `.gba` files
  to `/pc/roms` and point the ROM folder setting there instead.
- ISO9660 version suffixes (e.g. `GAME.GBA;1`) in filenames are stripped
  automatically from both display labels and file paths.

## Frontend Controls

### ROM Browser

| Button | Action |
|--------|--------|
| D-Pad / Analog | Move selection |
| A | Launch selected ROM |
| B | Return to loader |
| Y | Open settings |
| Start | Return to loader |

### Settings

| Button | Action |
|--------|--------|
| D-Pad Up/Down | Select row |
| D-Pad Left/Right | Adjust value |
| A | Save and return (on last row) |
| B | Cancel without saving |

Configurable options:

- **Performance** (`Accuracy` / `Balanced` / `Speed` — see Performance Profiles below)
- **Show FPS** (`On` / `Off` — overlays the measured frame rate during play)
- **Frame skip** (0–3 extra emulated frames per display frame)
- **Audio buffer** (2048 / 4096 / 8192 bytes — lower = less latency, higher = safer)
- **BIOS path** (`/cd/bios.bin` or `/pc/bios.bin`)
- **ROM folder** (`/pc/roms` or `/cd`)
- **Save folder** (`/pc/saves` or `/pc`)

Selecting a **Performance** profile rewrites the audio/video/frame-skip knobs
to that profile's preset; you can then fine-tune Frame skip and Audio buffer
afterward without losing the rest of the preset.

## Hardware Mapping (Gameplay)

| Dreamcast Button | GBA Button |
|-----------------|------------|
| A               | A          |
| B               | B          |
| X               | L          |
| Y               | R          |
| Start           | Start      |
| D (unused btn)  | Select     |
| D-Pad           | D-Pad      |
| Analog Stick    | D-Pad (dead zone: 32, ~25% of ±127 range) |

**Exit combo during gameplay**: Hold Start + A + B + X + Y for ~1 second to return to the ROM browser.

Errors and loading screens show on-screen text with path details. Press Start to dismiss fatal errors.

## Current Limitations

- **No save states** – save state UI is not implemented yet
- **No VMU saves** – filesystem saves only
- **No post-processing** – color correction and xBRZ upscaling are disabled (LCD ghosting is enabled only by the Accuracy profile)
- **Single-threaded** – the emulation loop runs on the main thread

## Architecture

```
src/platform/dreamcast/
├── CMakeLists.txt
└── src/
    ├── main.cc                    # Frontend loop + emulation sessions
    ├── dc_config.hh/cc            # Dreamcast settings (TOML)
    ├── dc_ui.hh/cc                # On-screen menus and overlays
    ├── dc_frontend.hh/cc          # ROM browser + settings menus
    ├── dc_rom_browser.hh/cc       # ROM directory scanning
    ├── dc_paths.hh                # Save path helpers
    └── device/
        ├── dc_video_device.hh/cc  # PVR framebuffer video output
        ├── dc_audio_device.hh/cc  # KOS snd_stream audio output
        └── dc_input.hh/cc         # Maple controller input polling
```

The Dreamcast backend reuses the core emulator library (`nba`) and the platform-core library (loaders, config, frame limiter) while providing its own device implementations.

## Performance Notes

The Dreamcast SH4 at 200 MHz is significantly slower than modern desktop CPUs. Expect performance challenges with cycle-accurate GBA emulation. Built-in tuning options:

- Performance profile (settings menu)
- Frame skip (settings menu)
- Audio buffer size (settings menu)
- SH4-specific compiler flags (`-m4-single-only` is already used)

### Performance Profiles

The **Performance** setting selects a coherent preset of the accuracy/speed
knobs. Pick the highest-fidelity profile a given game can sustain at full speed.

| Profile      | Audio mixing   | Interpolation | Frame skip | Audio buffer | LCD ghosting |
|--------------|----------------|---------------|------------|--------------|--------------|
| **Accuracy** | Native (LLE)   | Sinc-64       | 0          | 8192         | On           |
| **Balanced** | Native (LLE)   | Cosine        | 0          | 4096         | Off          |
| **Speed**    | MP2K HLE       | Cosine        | 1          | 8192         | Off          |

- **Accuracy** – closest to real GBA behavior; best for light 2D titles that
  already hold full speed and benefit from accurate audio.
- **Balanced** (default) – native audio with cheap interpolation and no frame
  skipping. Good fidelity with CPU headroom on most games.
- **Speed** – HLE audio bypasses the GBA sound CPU and one skipped frame
  reclaims headroom for the heaviest titles (3D/Mode-7-heavy games).

Switching profiles overwrites Frame skip and Audio buffer; adjust those rows
afterward to fine-tune within a profile.

### FPS Overlay / Benchmarking

Enable **Show FPS** in settings to overlay the measured display frame rate in
the top-left corner during play. The reading is averaged once per second by the
frame limiter. A title running at full speed reports ~59.7 FPS (the GBA's native
rate); sustained readings below that indicate the SH4 cannot keep up at the
current profile.

### Repeatable Benchmark Workflow

To compare profiles or measure a code change consistently:

1. Use the same retail Dreamcast hardware and the same BIOS dump for every run.
2. Pick a fixed in-game scene per ROM (e.g. a specific level intro or a heavy
   battle/effect screen) and reach it the same way each time.
3. Enable **Show FPS** and let the scene run untouched for ~30 seconds, then
   record the steady-state FPS reading.
4. Repeat across the Accuracy / Balanced / Speed profiles, changing only the
   profile between runs.
5. Note the lowest profile that holds ~59.7 FPS for that scene; that is the
   game's recommended profile in the compatibility table below.

A suggested benchmark set spans the GBA workload range: a light 2D title, a
sprite-heavy action title, a Mode-7 / pseudo-3D title, and an audio-heavy title.
Record actual ROMs used in `COMPATIBILITY.md` so results stay reproducible.

### Compatibility Tiers

Per-game results are tracked in [`COMPATIBILITY.md`](COMPATIBILITY.md) using
these tiers:

| Tier | Meaning |
|------|---------|
| **Playable** | Holds ~full speed on at least one profile with no game-breaking issues. |
| **Runs** | Boots and is playable but with noticeable slowdown or audio/video artifacts. |
| **Broken** | Fails to boot, hangs, or has issues that prevent normal play. |
