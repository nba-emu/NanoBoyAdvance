// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dc_config.hh"

#include <filesystem>

namespace fs = std::filesystem;

namespace nba {

inline auto EnsureDirectory(fs::path const& path) -> bool {
  if(path.empty()) {
    return false;
  }

  std::error_code error;
  if(fs::exists(path, error)) {
    return fs::is_directory(path, error);
  }

  return fs::create_directories(path, error);
}

inline auto GetSavePath(DreamcastConfig const& config, fs::path const& rom_path) -> fs::path {
  const auto stem = rom_path.stem().string();

  if(!config.save_folder.empty()) {
    return fs::path{config.save_folder} / (stem + ".sav");
  }

  const auto parent = rom_path.parent_path().string();
  if(parent.rfind("/cd", 0) == 0) {
    return fs::path{DreamcastConfig::kDefaultSaveFolder} / (stem + ".sav");
  }

  return rom_path.replace_extension(".sav");
}

} // namespace nba
