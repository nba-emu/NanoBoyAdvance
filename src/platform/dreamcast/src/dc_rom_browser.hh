// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dc_config.hh"

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace nba {

struct ROMEntry {
  fs::path path;
  std::string label;
};

struct ROMBrowser {
  static auto Scan(DreamcastConfig const& config) -> std::vector<ROMEntry>;
};

} // namespace nba
