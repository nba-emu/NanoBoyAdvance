/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <config/config.hpp>
#include <map>

namespace nba {

enum class GPIO {
  None,
  RTC
};

struct GameInfo {
  Config::BackupType backup_type = Config::BackupType::Detect;
  GPIO gpio  = GPIO::None;
  bool mirror = false;
};

extern const std::map<std::string, GameInfo> g_game_db;

} // namespace nba
