/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/config.hpp>
#include <string>
#include <toml.hpp>

namespace nba {

struct PlatformConfig : Config {
  std::string bios_path = "bios.bin";
  
  bool sync_to_audio = false;
  
  struct Cartridge {
    BackupType backup_type = BackupType::Detect;
    bool force_rtc = false;
    bool force_solar_sensor = false;
    u8 solar_sensor_level = 156;
  } cartridge;

  struct Video {
    bool fullscreen = false;
    int scale = 2;

    enum class Filter {
      Nearest,
      Linear,
      xBRZ
    } filter = Filter::Nearest;

    enum class Color {
      No,
      higan,
      AGB
    } color = Color::AGB;

    bool lcd_ghosting = true;

    struct Shader {
      std::string path_vs = "";
      std::string path_fs = "";
    } shader;
  } video;

  void Load(std::string const& path);
  void Save(std::string const& path);

protected:
  virtual void LoadCustomData(toml::value const& data) {}
  virtual void SaveCustomData(
    toml::basic_value<toml::preserve_comments>& data
  ) {};
};

} // namespace nba
