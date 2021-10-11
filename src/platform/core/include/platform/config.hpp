/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/config.hpp>
#include <string>

namespace nba {

struct PlatformConfig : Config {
  std::string bios_path = "bios.bin";
  
  bool sync_to_audio = false;
  
  BackupType backup_type = BackupType::Detect;
  
  bool force_rtc = false;

  struct Video {
    bool fullscreen = false;
    int scale = 2;

    struct Shader {
      std::string path_vs = "";
      std::string path_fs = "";
    } shader;
  } video;

  void Load(std::string const& path);
  void Save(std::string const& path);
};

} // namespace nba
