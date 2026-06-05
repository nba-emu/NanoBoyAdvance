// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

// Dreamcast frontend for NanoBoyAdvance.
// Single-threaded main loop: poll input → run one frame → render → audio pump.
// ROMs and BIOS are loaded from the filesystem (SD card via DreamSDK, or cd).

#include <nba/core.hh>
#include <platform/loader/bios.hh>
#include <platform/loader/rom.hh>
#include <platform/config.hh>
#include <platform/frame_limiter.hh>

#include <atom/logger/logger.hh>

#include "device/dc_video_device.hh"
#include "device/dc_audio_device.hh"
#include "device/dc_input.hh"

#include <memory>
#include <cstdio>
#include <exception>

#if defined(PLATFORM_DREAMCAST) && __has_include(<kos.h>)
#define NBA_DC_HAS_KOS 1
#include <kos.h>
#include <dc/maple/controller.h>
KOS_INIT_FLAGS(INIT_DEFAULT);
#else
#define NBA_DC_HAS_KOS 0
#endif

using namespace nba;

// Hardcoded paths for the Dreamcast filesystem.
// When using DreamSDK with an SD adapter these resolve to the SD card root.
static constexpr const char* kBIOSPath = "/cd/bios.bin";
static constexpr const char* kROMPath  = "/cd/rom.gba";
static constexpr const char* kSavePath = "/pc/rom.sav";

static constexpr float kGBAFrameRate =
  static_cast<float>(16777216) / static_cast<float>(280896);

#if NBA_DC_HAS_KOS
static void WaitForStartToExit() {
  std::printf("Press Start to return to the loader.\n");

  while(true) {
    maple_device_t* cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
    if(cont) {
      cont_state_t* state = (cont_state_t*)maple_dev_status(cont);
      if(state && (state->buttons & CONT_START)) {
        break;
      }
    }

    vid_waitvbl();
  }
}
#endif

static auto FailFatal(DCVideoDevice& video, const char* message) -> int {
  std::printf("Error: %s\n", message);
  video.ShowFatalError(message);

#if NBA_DC_HAS_KOS
  WaitForStartToExit();
#endif

  return 1;
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  std::printf("NanoBoyAdvance Dreamcast Edition\n");
  std::printf("Initializing...\n");

  auto video_device = std::make_shared<DCVideoDevice>();
  if(!video_device->Initialize()) {
    std::printf("Error: failed to initialize video device.\n");
    return 1;
  }

  // Validate media before opening audio or creating the core.
  auto bios_result = BIOSLoader::Validate(kBIOSPath);
  if(bios_result != BIOSLoader::Result::Success) {
    char message[128];
    std::snprintf(message, sizeof(message), "%s\n%s",
                  BIOSLoader::Describe(bios_result), kBIOSPath);
    return FailFatal(*video_device, message);
  }

  auto rom_result = ROMLoader::Validate(kROMPath);
  if(rom_result != ROMLoader::Result::Success) {
    char message[128];
    std::snprintf(message, sizeof(message), "%s\n%s",
                  ROMLoader::Describe(rom_result), kROMPath);
    return FailFatal(*video_device, message);
  }

  // --- Create platform config with Dreamcast-appropriate defaults ---
  auto config = std::make_shared<PlatformConfig>();
  config->audio.mp2k_hle_enable = false; // Disable HLE audio (too expensive)
  config->audio.interpolation = Config::Audio::Interpolation::Cosine; // Lightest resampler
  config->video.filter = PlatformConfig::Video::Filter::Nearest;
  config->video.color  = PlatformConfig::Video::Color::No; // Skip color correction
  config->video.lcd_ghosting = false; // Skip LCD ghosting (no shader support)
  config->video_dev = video_device;

  auto audio_device = std::make_shared<DCAudioDevice>();
  config->audio_dev = audio_device;

  // --- Create emulator core ---
  auto core = CreateCore(config);
  if(!core) {
    return FailFatal(*video_device, "Failed to create emulator core");
  }

  if(!audio_device->IsOpened()) {
    return FailFatal(*video_device, "Audio device failed to open");
  }

  // --- Load BIOS ---
  bios_result = BIOSLoader::Load(core, kBIOSPath);
  if(bios_result != BIOSLoader::Result::Success) {
    char message[128];
    std::snprintf(message, sizeof(message), "%s\n%s",
                  BIOSLoader::Describe(bios_result), kBIOSPath);
    return FailFatal(*video_device, message);
  }
  std::printf("BIOS loaded from %s\n", kBIOSPath);

  // --- Load ROM ---
  try {
    rom_result = ROMLoader::Load(core, kROMPath, kSavePath);
  } catch(const std::exception& exception) {
    char message[160];
    std::snprintf(message, sizeof(message), "Save file error\n%s", exception.what());
    return FailFatal(*video_device, message);
  }

  if(rom_result != ROMLoader::Result::Success) {
    char message[128];
    std::snprintf(message, sizeof(message), "%s\n%s",
                  ROMLoader::Describe(rom_result), kROMPath);
    return FailFatal(*video_device, message);
  }
  std::printf("ROM loaded from %s\n", kROMPath);
  std::printf("Save path: %s\n", kSavePath);

  // --- Reset core ---
  core->Reset();
  std::printf("Core reset. Entering main loop.\n");

  // --- Input handler ---
  DCInput input;
  FrameLimiter frame_limiter(kGBAFrameRate);

  // --- Main emulation loop (single-threaded) ---
  bool running = true;

  while(running) {
    frame_limiter.Run([&]() {
      if(input.PollInput(*core)) {
        running = false;
        return;
      }

      core->RunForOneFrame();
    }, [](float) {});

    // Pump audio stream (KOS needs periodic polling)
#if NBA_DC_HAS_KOS
    snd_stream_poll(SND_STREAM_INVALID);
    vid_waitvbl();
#endif
  }

  std::printf("Exiting NanoBoyAdvance.\n");

  // Destroy the core before exit so APU closes audio exactly once.
  core.reset();

  return 0;
}
