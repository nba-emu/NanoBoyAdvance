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

void config_toml_read (Config &config, std::string const& path);
void config_toml_write(Config &config, std::string const& path);

} // namespace nba
