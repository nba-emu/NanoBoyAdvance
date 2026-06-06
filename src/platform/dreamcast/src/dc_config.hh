// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <platform/config.hh>
#include <toml.hpp>
#include <string>

namespace nba {

struct DreamcastConfig : PlatformConfig {
  static constexpr const char* kDefaultConfigPath = "/pc/nba-dc.toml";
  static constexpr const char* kDefaultBIOSPath = "/cd/bios.bin";
  static constexpr const char* kDefaultROMFolder = "/pc/roms";
  static constexpr const char* kDefaultSaveFolder = "/pc/saves";

  // Performance profiles trade emulation accuracy for speed on the
  // fixed Dreamcast hardware budget. Selecting a profile applies a
  // coherent preset of the CPU/audio/video knobs below.
  enum class PerformanceProfile {
    Accuracy, // Highest fidelity, may not hold full speed on heavy games.
    Balanced, // Default. Good fidelity with headroom on most games.
    Speed     // Favors full speed: HLE audio, light interpolation, frame skip.
  };

  int frame_skip = 0;
  int audio_buffer_size = 4096;
  bool show_fps = false;
  bool allow_large_roms = false;
  PerformanceProfile performance_profile = PerformanceProfile::Balanced;
  std::string rom_folder = kDefaultROMFolder;
  std::string last_rom;

  void ApplyDefaults();
  // Applies a profile's preset to the CPU/audio/video knobs. Intended to be
  // called when the user explicitly switches profiles in the settings menu.
  void ApplyPerformanceProfile(PerformanceProfile profile);
  void LoadDreamcast(std::string const& path);
  void SaveDreamcast(std::string const& path);

  static auto ProfileName(PerformanceProfile profile) -> const char*;
  static auto ProfileFromName(std::string const& name, PerformanceProfile fallback)
    -> PerformanceProfile;

protected:
  void LoadCustomData(toml::value const& data) override;
  void SaveCustomData(toml::value& data) override;
};

} // namespace nba
