/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/config.hpp>
#include <map>

namespace nba {

enum class GPIODeviceType {
  None,
  RTC
};

struct GameInfo {
  Config::BackupType backup_type = Config::BackupType::Detect;
  GPIODeviceType gpio = GPIODeviceType::None;
  bool mirror = false;
};

extern const std::map<std::string, GameInfo> g_game_db;

} // namespace nba
