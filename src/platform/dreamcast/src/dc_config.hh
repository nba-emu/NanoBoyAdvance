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

  int frame_skip = 0;
  int audio_buffer_size = 4096;
  std::string rom_folder = kDefaultROMFolder;
  std::string last_rom;

  void ApplyDefaults();
  void LoadDreamcast(std::string const& path);
  void SaveDreamcast(std::string const& path);

protected:
  void LoadCustomData(toml::value const& data) override;
  void SaveCustomData(toml::value& data) override;
};

} // namespace nba
