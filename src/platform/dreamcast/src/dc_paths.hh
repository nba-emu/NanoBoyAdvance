// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dc_config.hh"

#include <filesystem>
#include <sys/stat.h>

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

// POSIX mkdir-based directory creator for Dreamcast /pc/ and /cd/ paths where
// std::filesystem may not work correctly through the KOS virtual filesystem.
// Returns true if the directory exists or was created successfully.
inline auto EnsureDirectoryPOSIX(std::string const& path) -> bool {
  if(path.empty()) {
    return false;
  }
  // Try to probe existence first with fopen (cheap, works on all KOS vfs).
  // If the path is a directory, fopen will fail; that's expected.
  // Use mkdir and accept EEXIST as success.
  if(::mkdir(path.c_str(), 0755) == 0) {
    return true; // created
  }
  // errno EEXIST means the directory already exists.
  return errno == EEXIST;
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

  auto save_path = rom_path;
  return save_path.replace_extension(".sav");
}

} // namespace nba
