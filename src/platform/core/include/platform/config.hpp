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
  void Load(std::string const& path);
  void Save(std::string const& path);
};

} // namespace nba
