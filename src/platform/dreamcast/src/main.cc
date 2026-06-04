// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

// Dreamcast frontend for NanoBoyAdvance.
// Single-threaded main loop: poll input → run one frame → render → audio pump.
// ROMs and BIOS are loaded from the filesystem (SD card via DreamSDK, or cd).

#include <nba/core.hh>
#include <platform/loader/bios.hh>
#include <platform/loader/rom.hh>
#include <platform/config.hh>

#include <atom/logger/logger.hh>

#include "device/dc_video_device.hh"
#include "device/dc_audio_device.hh"
#include "device/dc_input.hh"

#include <memory>
#include <cstdio>

#if defined(PLATFORM_DREAMCAST) && __has_include(<kos.h>)
#define NBA_DC_HAS_KOS 1
#include <kos.h>
KOS_INIT_FLAGS(INIT_DEFAULT);
#else
#define NBA_DC_HAS_KOS 0
#endif

using namespace nba;

// Hardcoded paths for the Dreamcast filesystem.
// When using DreamSDK with an SD adapter these resolve to the SD card root.
static constexpr const char* kBIOSPath = "/cd/bios.bin";
static constexpr const char* kROMPath  = "/cd/rom.gba";

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  std::printf("NanoBoyAdvance Dreamcast Edition\n");
  std::printf("Initializing...\n");

  // --- Create platform config with Dreamcast-appropriate defaults ---
  auto config = std::make_shared<PlatformConfig>();
  config->audio.mp2k_hle_enable = false; // Disable HLE audio (too expensive)
  config->audio.interpolation = Config::Audio::Interpolation::Cosine; // Lightest resampler
  config->video.filter = PlatformConfig::Video::Filter::Nearest;
  config->video.color  = PlatformConfig::Video::Color::No; // Skip color correction
  config->video.lcd_ghosting = false; // Skip LCD ghosting (no shader support)

  // --- Create and attach Dreamcast-specific devices ---
  auto video_device = std::make_shared<DCVideoDevice>();
  if(!video_device->Initialize()) {
    std::printf("Error: failed to initialize video device.\n");
    return 1;
  }
  config->video_dev = video_device;

  auto audio_device = std::make_shared<DCAudioDevice>();
  config->audio_dev = audio_device;

  // --- Create emulator core ---
  auto core = CreateCore(config);
  if(!core) {
    std::printf("Error: failed to create emulator core.\n");
    return 1;
  }

  // --- Load BIOS ---
  auto bios_result = BIOSLoader::Load(core, kBIOSPath);
  if(bios_result != BIOSLoader::Result::Success) {
    std::printf("Error: failed to load BIOS from %s (error %d).\n",
                kBIOSPath, (int)bios_result);
    return 1;
  }
  std::printf("BIOS loaded from %s\n", kBIOSPath);

  // --- Load ROM ---
  auto rom_result = ROMLoader::Load(core, kROMPath);
  if(rom_result != ROMLoader::Result::Success) {
    std::printf("Error: failed to load ROM from %s (error %d).\n",
                kROMPath, (int)rom_result);
    return 1;
  }
  std::printf("ROM loaded from %s\n", kROMPath);

  // --- Reset core ---
  core->Reset();
  std::printf("Core reset. Entering main loop.\n");

  // --- Input handler ---
  DCInput input;

  // --- Main emulation loop (single-threaded) ---
  bool running = true;

  while(running) {
    // Poll Dreamcast controller input
    input.PollInput(*core);

    // Run one GBA frame
    core->RunForOneFrame();

    // Pump audio stream (KOS needs periodic polling)
#if NBA_DC_HAS_KOS
    snd_stream_poll(SND_STREAM_INVALID); // poll all streams
#endif

    // Check for exit condition (Start + A + B + X + Y held simultaneously)
#if NBA_DC_HAS_KOS
    maple_device_t* cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if(cont) {
      cont_state_t* state = (cont_state_t*)maple_dev_status(cont);
      if(state) {
        const uint32 exit_combo = CONT_START | CONT_A | CONT_B | CONT_X | CONT_Y;
        if((state->buttons & exit_combo) == exit_combo) {
          running = false;
        }
      }
    }
#endif
  }

  std::printf("Exiting NanoBoyAdvance.\n");

  // Clean up audio before exit
  audio_device->Close();

  return 0;
}
