/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <filesystem>
#include <nba/core.hpp>
#include <string>

namespace fs = std::filesystem;

namespace nba {

struct SaveStateLoader {
  enum class Result {
    CannotFindFile,
    CannotOpenFile,
    BadImage,
    UnsupportedVersion,
    Success
  };

  static auto Load(
    std::unique_ptr<CoreBase>& core,
    fs::path const& path
  ) -> Result;

private:
  static auto Validate(SaveState const& save_state) -> Result;
};

} // namespace nba
