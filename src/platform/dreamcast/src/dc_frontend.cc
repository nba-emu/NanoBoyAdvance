// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dc_frontend.hh"

#include "dc_paths.hh"
#include "dc_rom_browser.hh"

#include <algorithm>
#include <array>
#include <cstdio>

#if NBA_DC_HAS_KOS
#include <dc/video.h>
#endif

namespace nba {

namespace {

struct SettingRow {
  const char* label;
  std::string (*getter)(DreamcastConfig const&);
  void (*adjust)(DreamcastConfig&, int direction);
};

auto PerformanceLabel(DreamcastConfig const& config) -> std::string {
  return DreamcastConfig::ProfileName(config.performance_profile);
}

auto ShowFpsLabel(DreamcastConfig const& config) -> std::string {
  return config.show_fps ? "On" : "Off";
}

auto AllowLargeRomsLabel(DreamcastConfig const& config) -> std::string {
  return config.allow_large_roms ? "On" : "Off";
}

auto FrameSkipLabel(DreamcastConfig const& config) -> std::string {
  return std::to_string(config.frame_skip);
}

auto AudioBufferLabel(DreamcastConfig const& config) -> std::string {
  return std::to_string(config.audio_buffer_size);
}

auto BiosPathLabel(DreamcastConfig const& config) -> std::string {
  return config.bios_path;
}

auto ROMFolderLabel(DreamcastConfig const& config) -> std::string {
  return config.rom_folder;
}

auto SaveFolderLabel(DreamcastConfig const& config) -> std::string {
  return config.save_folder;
}

void AdjustPerformance(DreamcastConfig& config, int direction) {
  static constexpr std::array<DreamcastConfig::PerformanceProfile, 3> kProfiles{
    DreamcastConfig::PerformanceProfile::Accuracy,
    DreamcastConfig::PerformanceProfile::Balanced,
    DreamcastConfig::PerformanceProfile::Speed
  };
  const auto current = std::find(kProfiles.begin(), kProfiles.end(), config.performance_profile);
  int index = current == kProfiles.end() ? 1 : static_cast<int>(current - kProfiles.begin());
  index = std::clamp(index + direction, 0, static_cast<int>(kProfiles.size()) - 1);
  config.ApplyPerformanceProfile(kProfiles[index]);
}

void AdjustShowFps(DreamcastConfig& config, int direction) {
  (void)direction;
  config.show_fps = !config.show_fps;
}

void AdjustAllowLargeRoms(DreamcastConfig& config, int direction) {
  (void)direction;
  config.allow_large_roms = !config.allow_large_roms;
}

void AdjustFrameSkip(DreamcastConfig& config, int direction) {
  config.frame_skip = std::clamp(config.frame_skip + direction, 0, 3);
}

void AdjustAudioBuffer(DreamcastConfig& config, int direction) {
  static constexpr std::array<int, 3> kSizes{2048, 4096, 8192};
  auto current = std::find(kSizes.begin(), kSizes.end(), config.audio_buffer_size);
  if(current == kSizes.end()) {
    current = kSizes.begin() + 1;
  }

  const int index = static_cast<int>(current - kSizes.begin());
  const int next = std::clamp(index + direction, 0, static_cast<int>(kSizes.size()) - 1);
  config.audio_buffer_size = kSizes[next];
}

void AdjustBiosPath(DreamcastConfig& config, int direction) {
  static constexpr std::array<const char*, 2> kPaths{"/cd/bios.bin", "/pc/bios.bin"};
  const auto current = std::find(kPaths.begin(), kPaths.end(), config.bios_path);
  int index = current == kPaths.end() ? 0 : static_cast<int>(current - kPaths.begin());
  index = (index + direction + static_cast<int>(kPaths.size())) % static_cast<int>(kPaths.size());
  config.bios_path = kPaths[index];
}

void AdjustROMFolder(DreamcastConfig& config, int direction) {
  static constexpr std::array<const char*, 2> kPaths{"/pc/roms", "/cd"};
  const auto current = std::find(kPaths.begin(), kPaths.end(), config.rom_folder);
  int index = current == kPaths.end() ? 0 : static_cast<int>(current - kPaths.begin());
  index = (index + direction + static_cast<int>(kPaths.size())) % static_cast<int>(kPaths.size());
  config.rom_folder = kPaths[index];
}

void AdjustSaveFolder(DreamcastConfig& config, int direction) {
  static constexpr std::array<const char*, 2> kPaths{"/pc/saves", "/pc"};
  const auto current = std::find(kPaths.begin(), kPaths.end(), config.save_folder);
  int index = current == kPaths.end() ? 0 : static_cast<int>(current - kPaths.begin());
  index = (index + direction + static_cast<int>(kPaths.size())) % static_cast<int>(kPaths.size());
  config.save_folder = kPaths[index];
}

static constexpr SettingRow kSettings[] {
  { "Performance", PerformanceLabel, AdjustPerformance },
  { "Show FPS", ShowFpsLabel, AdjustShowFps },
  { "Large ROMs (>8 MiB)", AllowLargeRomsLabel, AdjustAllowLargeRoms },
  { "Frame skip", FrameSkipLabel, AdjustFrameSkip },
  { "Audio buffer", AudioBufferLabel, AdjustAudioBuffer },
  { "BIOS path", BiosPathLabel, AdjustBiosPath },
  { "ROM folder", ROMFolderLabel, AdjustROMFolder },
  { "Save folder", SaveFolderLabel, AdjustSaveFolder }
};

auto BuildMenuItems(std::vector<ROMEntry> const& entries) -> std::vector<std::string> {
  std::vector<std::string> items;
  items.reserve(entries.size());

  for(auto const& entry : entries) {
    items.push_back(entry.label);
  }

  return items;
}

} // namespace

auto DCSettingsMenu::Run(
  DCUI& ui,
  DCInput& input,
  DreamcastConfig& config
) -> void {
  int selection = 0;
  const int item_count = static_cast<int>(std::size(kSettings));

  while(true) {
    std::vector<std::string> items;
    items.reserve(item_count + 1);

    for(auto const& setting : kSettings) {
      items.push_back(std::string{setting.label} + ": " + setting.getter(config));
    }
    items.emplace_back("Save and return");

    ui.DrawMenu("Settings", items, selection, 0);

    DCMenuInput menu;
    input.PollMenu(menu);

    if(menu.up) {
      selection = (selection + item_count) % (item_count + 1);
    } else if(menu.down) {
      selection = (selection + 1) % (item_count + 1);
    } else if(menu.left && selection < item_count) {
      kSettings[selection].adjust(config, -1);
    } else if(menu.right && selection < item_count) {
      kSettings[selection].adjust(config, 1);
    } else if(menu.confirm) {
      if(selection == item_count) {
        config.SaveDreamcast(DreamcastConfig::kDefaultConfigPath);
        return;
      }
    } else if(menu.cancel) {
      return;
    }

#if NBA_DC_HAS_KOS
    vid_waitvbl();
#endif
  }
}

auto DCFrontend::Run(
  DCUI& ui,
  DCInput& input,
  DreamcastConfig& config,
  std::vector<ROMEntry> const& entries
) -> Result {
  if(entries.empty()) {
    ui.ShowMessage(
      "No ROMs Found",
      "Place .gba files in /pc/roms\nor on the disc root (/cd).\n\nPress Start to open settings.",
      input,
      false
    );

    while(true) {
      DCMenuInput menu;
      input.PollMenu(menu);
      if(menu.start) {
        DCSettingsMenu::Run(ui, input, config);
        return Result{Action::OpenSettings, {}};
      }
      if(menu.settings) {
        DCSettingsMenu::Run(ui, input, config);
      }

#if NBA_DC_HAS_KOS
      vid_waitvbl();
#endif
    }
  }

  auto menu_items = BuildMenuItems(entries);
  int selection = 0;
  int scroll_offset = 0;

  if(!config.last_rom.empty()) {
    for(int i = 0; i < static_cast<int>(entries.size()); i++) {
      if(entries[i].path == config.last_rom) {
        selection = i;
        break;
      }
    }
  }

  static constexpr int kVisibleRows = 10;

  while(true) {
    ui.DrawMenu("Select ROM", menu_items, selection, scroll_offset);

    DCMenuInput menu;
    input.PollMenu(menu);

    if(menu.up) {
      selection = (selection + static_cast<int>(entries.size()) - 1) % static_cast<int>(entries.size());
    } else if(menu.down) {
      selection = (selection + 1) % static_cast<int>(entries.size());
    } else if(menu.confirm) {
      config.last_rom = entries[selection].path.string();
      return Result{Action::LaunchROM, entries[selection].path};
    } else if(menu.settings) {
      DCSettingsMenu::Run(ui, input, config);
      return Result{Action::OpenSettings, {}};
    } else if(menu.start) {
      return Result{Action::ReturnToLoader, {}};
    } else if(menu.cancel) {
      return Result{Action::ReturnToLoader, {}};
    }

    if(selection < scroll_offset) {
      scroll_offset = selection;
    } else if(selection >= scroll_offset + kVisibleRows) {
      scroll_offset = selection - kVisibleRows + 1;
    }

#if NBA_DC_HAS_KOS
    vid_waitvbl();
#endif
  }
}

} // namespace nba
