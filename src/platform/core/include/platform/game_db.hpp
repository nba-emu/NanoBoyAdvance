/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/config.hpp>
#include <map>

namespace nba {

enum class GPIODeviceType {
  None = 0,
  RTC = 1,
  SolarSensor = 2
};

struct GameInfo {
  Config::BackupType backup_type = Config::BackupType::Detect;
  GPIODeviceType gpio = GPIODeviceType::None;
  bool mirror = false;
};

extern const std::map<std::string, GameInfo> g_game_db;

constexpr GPIODeviceType operator|(GPIODeviceType lhs, GPIODeviceType rhs) {
  return (GPIODeviceType)((int)lhs | (int)rhs);
}

constexpr int operator&(GPIODeviceType lhs, GPIODeviceType rhs) {
  return (int)lhs & (int)rhs;
}

} // namespace nba
