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
cp your_rom.gba  cd_root/rom.gba

# Create the ISO
mkisofs -C 0,11702 -V "NBA_DC" -G IP.BIN -l -o nba-dc.iso cd_root/
```

## File Layout on Disc

The Dreamcast frontend expects:

| File | Path | Description |
|------|------|-------------|
| GBA BIOS | `/cd/bios.bin` | Required. Dump from real hardware or use unofficial BIOS |
| GBA ROM  | `/cd/rom.gba`  | The ROM to run |

## Hardware Mapping

| Dreamcast Button | GBA Button |
|-----------------|------------|
| A               | A          |
| B               | B          |
| X               | L          |
| Y               | R          |
| Start           | Start      |
| D (unused btn)  | Select     |
| D-Pad           | D-Pad      |
| Analog Stick    | D-Pad (with dead zone) |

**Exit combo**: Hold Start + A + B + X + Y simultaneously to exit.

## Current Limitations

- **No save states** – save state UI is not implemented yet
- **No ROM browser** – a single ROM path is hardcoded; change `kROMPath` in `main.cc`
- **No post-processing** – color correction, xBRZ upscaling, and LCD ghosting are disabled
- **No HLE audio** – MP2K HLE is disabled for performance reasons
- **Single-threaded** – the emulation loop runs on the main thread
- **No VMU save support** – backup saves go to the filesystem only

## Architecture

```
src/platform/dreamcast/
├── CMakeLists.txt
└── src/
    ├── main.cc                    # Entry point + main loop
    └── device/
        ├── dc_video_device.hh/cc  # PVR framebuffer video output
        ├── dc_audio_device.hh/cc  # KOS snd_stream audio output
        └── dc_input.hh/cc         # Maple controller input polling
```

The Dreamcast backend reuses the core emulator library (`nba`) and the platform-core library (loaders, config, frame limiter) while providing its own device implementations.

## Performance Notes

The Dreamcast SH4 at 200 MHz is significantly slower than modern desktop CPUs. Expect performance challenges with cycle-accurate GBA emulation. Potential future optimizations:

- Frame skipping
- Audio buffer tuning
- SH4-specific compiler flags (`-m4-single-only` is already used)
- Selective accuracy trade-offs for performance-critical paths
