// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/config.hh>
#include <toml.hpp>
#include <string>

namespace nba {

struct PlatformConfig : Config {
  std::string bios_path = "bios.bin";
  std::string save_folder = "";

  struct Cartridge {
    BackupType backup_type = BackupType::Detect;
    bool force_rtc = true;
    bool force_solar_sensor = false;
    u8 solar_sensor_level = 23;
  } cartridge;

  struct Video {
    enum class Filter {
      Nearest,
      Linear,
      Sharp,
      xBRZ,
      Lcd1x
    } filter = Filter::Nearest;

    enum class Color {
      No,
      higan,
      AGB
    } color = Color::AGB;

    bool lcd_ghosting = true;
  } video;

  void Load(std::string const& path);
  void Save(std::string const& path);

protected:
  // Applies an already-parsed TOML document onto this config.  Split out from
  // Load(path) so callers that obtained the document another way (e.g. reading
  // the file through a platform-specific API) can reuse the same mapping.
  void LoadFromData(toml::value const& data);

  // Populates a TOML document from this config.  Split out from Save(path) so
  // callers can serialize and write the document through a platform-specific
  // API instead of std::ofstream.
  void SaveToData(toml::value& data);

  virtual void LoadCustomData(toml::value const& data) {}
  virtual void SaveCustomData(toml::value& data) {}
};

} // namespace nba
