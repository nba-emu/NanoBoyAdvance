/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <common/log.hpp>
#include <string>

#include "config.hpp"

namespace nba {

void config_toml_read (Config &config, std::string const& path);
void config_toml_write(Config &config, std::string const& path);

} // namespace nba
