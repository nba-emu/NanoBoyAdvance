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
  size_t size = 0;
  // True when the ROM is larger than the stock 8 MiB limit and the current
  // configuration (Large ROMs setting / detected 32 MB RAM) cannot load it.
  // The browser marks these and refuses to launch them with an explanation.
  bool needs_large_roms = false;
};

struct ROMBrowser {
  static auto Scan(DreamcastConfig const& config) -> std::vector<ROMEntry>;
};

} // namespace nba
