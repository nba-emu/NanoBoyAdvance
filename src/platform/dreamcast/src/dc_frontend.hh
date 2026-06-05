// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dc_config.hh"
#include "dc_rom_browser.hh"
#include "dc_ui.hh"
#include "device/dc_input.hh"

#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

namespace nba {

struct DCFrontend {
  enum class Action {
    LaunchROM,
    OpenSettings,
    ReturnToLoader
  };

  struct Result {
    Action action = Action::ReturnToLoader;
    fs::path rom_path;
  };

  static auto Run(
    DCUI& ui,
    DCInput& input,
    DreamcastConfig& config,
    std::vector<ROMEntry> const& entries
  ) -> Result;
};

struct DCSettingsMenu {
  static auto Run(
    DCUI& ui,
    DCInput& input,
    DreamcastConfig& config
  ) -> void;
};

} // namespace nba
