// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

// Dreamcast frontend for NanoBoyAdvance.

#include <nba/core.hh>
#include <platform/loader/bios.hh>
#include <platform/loader/rom.hh>
#include <platform/frame_limiter.hh>

#include "dc_config.hh"
#include "dc_frontend.hh"
#include "dc_paths.hh"
#include "dc_rom_browser.hh"
#include "dc_ui.hh"
#include "device/dc_video_device.hh"
#include "device/dc_audio_device.hh"
#include "device/dc_input.hh"

#include <cstdio>
#include <exception>
#include <memory>
#include <string>

#if defined(PLATFORM_DREAMCAST) && __has_include(<kos.h>)
#define NBA_DC_HAS_KOS 1
#include <kos.h>
KOS_INIT_FLAGS(INIT_DEFAULT);
#else
#define NBA_DC_HAS_KOS 0
#endif

using namespace nba;

static constexpr float kGBAFrameRate =
  static_cast<float>(16777216) / static_cast<float>(280896);

static auto LoadEmulator(
  DCUI& ui,
  DCInput& input,
  std::shared_ptr<DreamcastConfig> config,
  std::shared_ptr<DCVideoDevice> video_device,
  fs::path const& rom_path
) -> bool {
  const auto bios_path = config->bios_path;

  auto bios_result = BIOSLoader::Validate(bios_path);
  if(bios_result != BIOSLoader::Result::Success) {
    char message[160];
    std::snprintf(message, sizeof(message), "%s\n%s",
                  BIOSLoader::Describe(bios_result), bios_path.c_str());
    ui.ShowFatalError(message, input);
    return false;
  }

  auto rom_result = ROMLoader::Validate(rom_path);
  if(rom_result != ROMLoader::Result::Success) {
    char message[160];
    std::snprintf(message, sizeof(message), "%s\n%s",
                  ROMLoader::Describe(rom_result), rom_path.c_str());
    ui.ShowFatalError(message, input);
    return false;
  }

  if(!EnsureDirectory(config->save_folder)) {
    ui.ShowFatalError("Could not create save folder", input);
    return false;
  }

  const auto save_path = GetSavePath(*config, rom_path);

  ui.ClearScreen();
  ui.DrawTitle("Loading");
  ui.DrawTextMultiline(48, 120, std::string{"BIOS: "} + bios_path + "\nROM: " + rom_path.string() +
                                   "\nSave: " + save_path.string());
  ui.DrawStatusBar("Loading game data...");
  ui.Present();

  auto audio_device = std::make_shared<DCAudioDevice>();
  audio_device->SetBufferSize(config->audio_buffer_size);
  config->audio_dev = audio_device;
  config->video_dev = video_device;

  auto core = CreateCore(config);
  if(!core || !audio_device->IsOpened()) {
    ui.ShowFatalError("Failed to initialize emulator core", input);
    return false;
  }

  bios_result = BIOSLoader::Load(core, bios_path);
  if(bios_result != BIOSLoader::Result::Success) {
    char message[160];
    std::snprintf(message, sizeof(message), "%s\n%s",
                  BIOSLoader::Describe(bios_result), bios_path.c_str());
    ui.ShowFatalError(message, input);
    return false;
  }

  try {
    rom_result = ROMLoader::Load(
      core,
      rom_path,
      save_path,
      config->cartridge.backup_type
    );
  } catch(const std::exception& exception) {
    char message[192];
    std::snprintf(message, sizeof(message), "Save file error\n%s", exception.what());
    ui.ShowFatalError(message, input);
    return false;
  }

  if(rom_result != ROMLoader::Result::Success) {
    char message[160];
    std::snprintf(message, sizeof(message), "%s\n%s",
                  ROMLoader::Describe(rom_result), rom_path.c_str());
    ui.ShowFatalError(message, input);
    return false;
  }

  core->Reset();

  FrameLimiter frame_limiter(kGBAFrameRate);
  bool running = true;

  while(running) {
    frame_limiter.Run([&]() {
      if(input.PollInput(*core)) {
        running = false;
        return;
      }

      for(int skip = 0; skip <= config->frame_skip; skip++) {
        core->RunForOneFrame();
      }
    }, [](float) {});

#if NBA_DC_HAS_KOS
    snd_stream_poll(SND_STREAM_INVALID);

    if(input.IsExitHintActive()) {
      video_device->DrawOverlay("Hold Start+A+B+X+Y to exit");
    }

    vid_waitvbl();
#endif
  }

  ui.ClearScreen();
  ui.DrawOverlay("Returning to menu...");
  core.reset();
  return true;
}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  std::printf("NanoBoyAdvance Dreamcast Edition\n");

  auto video_device = std::make_shared<DCVideoDevice>();
  if(!video_device->Initialize()) {
    std::printf("Error: failed to initialize video device.\n");
    return 1;
  }

  DCUI ui{*video_device};
  DCInput input;

  auto config = std::make_shared<DreamcastConfig>();
  config->LoadDreamcast(DreamcastConfig::kDefaultConfigPath);
  EnsureDirectory(config->save_folder);
  EnsureDirectory(config->rom_folder);

  ui.ClearScreen();
  ui.DrawTitle("NanoBoyAdvance");
  ui.DrawTextCentered(120, "Dreamcast Edition");
  ui.DrawStatusBar("Loading frontend...");
  ui.Present();

  while(true) {
    auto entries = ROMBrowser::Scan(*config);
    auto frontend_result = DCFrontend::Run(ui, input, *config, entries);

    if(frontend_result.action == DCFrontend::Action::ReturnToLoader) {
      ui.ShowMessage("Goodbye", "Returning to loader.", input, false);
      break;
    }

    if(frontend_result.action == DCFrontend::Action::OpenSettings) {
      continue;
    }

    if(frontend_result.action == DCFrontend::Action::LaunchROM) {
      LoadEmulator(ui, input, config, video_device, frontend_result.rom_path);
    }
  }

  return 0;
}
