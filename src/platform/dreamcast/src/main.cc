// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

// Dreamcast frontend for NanoBoyAdvance.

#include <nba/core.hh>
#include <platform/loader/bios.hh>
#include <platform/loader/rom.hh>
#include <platform/frame_limiter.hh>

#include "dc_config.hh"
#include "dc_frontend.hh"
#include "dc_memory.hh"
#include "dc_paths.hh"
#include "dc_rom_browser.hh"
#include "dc_ui.hh"
#include "device/dc_video_device.hh"
#include "device/dc_audio_device.hh"
#include "device/dc_input.hh"
#include "open_bios.hh"

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

auto BIOSLoader::LoadEmbedded(std::unique_ptr<CoreBase>& core) -> Result {
  std::vector<u8> file_data(kOpenBIOS, kOpenBIOS + 16384);
  core->Attach(file_data);
  return Result::Success;
}

static constexpr float kGBAFrameRate =
  static_cast<float>(16777216) / static_cast<float>(280896);
static constexpr size_t kMaxGBAROMSize = 32 * 1024 * 1024;

static auto IsDreamcastVirtualPath(fs::path const& path) -> bool {
  const auto path_string = path.string();
  return path_string.rfind("/cd/", 0) == 0 || path_string.rfind("/pc/", 0) == 0;
}

static auto FormatROMSize(size_t size) -> std::string {
  char message[48];
  std::snprintf(
    message,
    sizeof(message),
    "%lu bytes (%lu MiB)",
    static_cast<unsigned long>(size),
    static_cast<unsigned long>(size / (1024 * 1024))
  );
  return message;
}

static auto LoadEmulator(
  DCUI& ui,
  DCInput& input,
  std::shared_ptr<DreamcastConfig> config,
  std::shared_ptr<DCVideoDevice> video_device,
  fs::path const& rom_path
) -> bool {
  const auto bios_path = config->bios_path;

  auto breadcrumb = [&](const char* phase, std::string const& detail = {}) {
    std::printf("[NBA-DC] %s", phase);
    if(!detail.empty()) {
      std::printf(": %s", detail.c_str());
    }
    std::printf("\n");
    std::fflush(stdout);

    ui.ClearScreen();
    ui.DrawTitle("Debug");
    ui.DrawTextMultiline(48, 96, std::string{phase} + (detail.empty() ? "" : "\n" + detail));
    ui.DrawStatusBar("Launching ROM...");
    ui.Present();
  };

  breadcrumb("Phase 1: BIOS check", bios_path);
  auto bios_result = BIOSLoader::Validate(bios_path);
  bool using_embedded_bios = false;

  if(bios_result != BIOSLoader::Result::Success) {
    char msg[160];
    std::snprintf(msg, sizeof(msg), "%s not found - using OpenBIOS", bios_path.c_str());
    ui.ClearScreen();
    ui.DrawTitle("OpenBIOS");
    ui.DrawTextMultiline(48, 120, msg);
    ui.DrawStatusBar("Continuing with built-in BIOS...");
    ui.Present();
    using_embedded_bios = true;
  }

  auto rom_result = ROMLoader::Result::Success;
  size_t rom_size = 0;
  auto size_result = ROMLoader::GetFileSize(rom_path, rom_size);
  if(size_result != ROMLoader::Result::Success) {
    char message[160];
    std::snprintf(message, sizeof(message), "%s\n%s",
                  ROMLoader::Describe(size_result), rom_path.c_str());
    ui.ShowFatalError(message, input);
    return false;
  }

  if(rom_size > kMaxGBAROMSize) {
    char message[192];
    std::snprintf(
      message,
      sizeof(message),
      "ROM is too large for GBA\n%s\n%s",
      FormatROMSize(rom_size).c_str(),
      rom_path.c_str()
    );
    ui.ShowFatalError(message, input);
    return false;
  }

#if NBA_DC_HAS_KOS
  if(IsDreamcastVirtualPath(rom_path) &&
      rom_size > kStockDreamcastMaxROMSize &&
      !CanLoadLargeROM(*config)) {
    char message[320];
    std::snprintf(
      message,
      sizeof(message),
      "ROM exceeds 8 MiB stock limit\n%s\n%s\n\nEnable Large ROMs in Settings\nor use a 32 MB RAM mod.",
      FormatROMSize(rom_size).c_str(),
      rom_path.c_str()
    );
    ui.ShowFatalError(message, input);
    return false;
  }
#endif

#if NBA_DC_HAS_KOS
  if(IsDreamcastVirtualPath(rom_path)) {
    std::string phase2_detail = FormatROMSize(rom_size);
#if NBA_DC_HAS_ARCH
    phase2_detail += HasExtendedRAM() ? "\nSystem RAM: 32 MB" : "\nSystem RAM: 16 MB";
#endif
    breadcrumb("Phase 2: ROM size precheck", phase2_detail);
  } else
#endif
  {
    breadcrumb("Phase 2: ROM precheck", FormatROMSize(rom_size));
    rom_result = ROMLoader::Validate(rom_path);
  }
  if(rom_result != ROMLoader::Result::Success) {
    char message[160];
    std::snprintf(message, sizeof(message), "%s\n%s",
                  ROMLoader::Describe(rom_result), rom_path.c_str());
    ui.ShowFatalError(message, input);
    return false;
  }

  // Save directory may not be writable on FlyCast; continue anyway

  const auto save_path = GetSavePath(*config, rom_path);

  // Ensure the save directory exists before creating backup files.
  // EnsureDirectoryPOSIX uses POSIX mkdir which works through the KOS
  // virtual filesystem on both real hardware and Flycast.
  {
    const auto save_dir = save_path.parent_path().string();
    if(!save_dir.empty()) {
      const bool dir_ok = EnsureDirectoryPOSIX(save_dir);
      breadcrumb("Phase 3: Save path",
                 save_path.string() + (dir_ok ? " [dir ok]" : " [dir failed]"));
    } else {
      breadcrumb("Phase 3: Save path", save_path.string());
    }
  }

  ui.ClearScreen();
  ui.DrawTitle("Loading");
  ui.DrawTextMultiline(48, 120, std::string{"BIOS: "} + bios_path + "\nROM: " + rom_path.string() +
                                   "\nSave: " + save_path.string());
  ui.DrawStatusBar("Loading game data...");
  ui.Present();

  breadcrumb("Phase 4: Audio device");
  auto audio_device = std::make_shared<DCAudioDevice>();
  audio_device->SetBufferSize(config->audio_buffer_size);
  config->audio_dev = audio_device;
  config->video_dev = video_device;

  breadcrumb("Phase 5: Core create");
  auto core = CreateCore(config);
  if(!core || !audio_device->IsOpened()) {
    ui.ShowFatalError("Failed to initialize emulator core", input);
    return false;
  }

  breadcrumb("Phase 6: BIOS load", using_embedded_bios ? "Embedded OpenBIOS" : bios_path);
  if(using_embedded_bios) {
    bios_result = BIOSLoader::LoadEmbedded(core);
  } else {
    bios_result = BIOSLoader::Load(core, bios_path);
  }
  if(bios_result != BIOSLoader::Result::Success) {
    char message[160];
    std::snprintf(message, sizeof(message), "%s\n%s",
                  BIOSLoader::Describe(bios_result), bios_path.c_str());
    ui.ShowFatalError(message, input);
    return false;
  }

  try {
    breadcrumb("Phase 7: ROM load", rom_path.string());
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

  breadcrumb("Phase 8: Core reset");
  core->Reset();
  breadcrumb("Phase 9: Enter frame loop");

  FrameLimiter frame_limiter(kGBAFrameRate);
  bool running = true;
  float measured_fps = 0.0f;
  int debug_frames = 0;

  while(running) {
    if(debug_frames < 3) {
      char message[64];
      std::snprintf(message, sizeof(message), "Frame %d before CPU", debug_frames);
      breadcrumb("Phase 10: First frames", message);
    }

    frame_limiter.Run([&]() {
      if(input.PollInput(*core)) {
        running = false;
        return;
      }

      for(int skip = 0; skip <= config->frame_skip; skip++) {
        core->RunForOneFrame();
      }
    }, [&](float fps) {
      measured_fps = fps;
    });

    if(debug_frames < 3) {
      char message[64];
      std::snprintf(message, sizeof(message), "Frame %d after CPU", debug_frames);
      breadcrumb("Phase 11: First frames", message);
      debug_frames++;
    }

#if NBA_DC_HAS_KOS
    snd_stream_poll(SND_STREAM_INVALID);

    if(config->show_fps) {
      // Persistent benchmark readout in the top-left margin (outside the
      // centered GBA frame, so the per-frame blit never overwrites it).
      // Fixed width keeps stale digits from lingering as the value changes.
      char fps_text[24];
      std::snprintf(fps_text, sizeof(fps_text), "FPS %5.1f", measured_fps);
      video_device->DrawText(8, 8, fps_text);
    }

    if(input.IsExitHintActive()) {
      video_device->DrawOverlay("Hold Start+A+B+X+Y to exit");
    }

    video_device->Present();
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

  // Draw UI first, then do FS operations (which may hang on FlyCast)
  ui.ClearScreen();
  ui.DrawTitle("NanoBoyAdvance");
  ui.DrawTextCentered(120, "Dreamcast Edition");
  ui.DrawStatusBar("Loading...");
  ui.Present();

  DCInput input;

  auto config = std::make_shared<DreamcastConfig>();
  config->ApplyDefaults();
  // Skip filesystem-dependent LoadDreamcast/EnsureDirectory that may hang on FlyCast

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
