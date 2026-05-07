/*
 * Copyright (C) 2026 Mireille Meyer
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <filesystem>

namespace fs = std::filesystem;

namespace nba {

struct BIOSLoader {
  enum class Result {
    CannotFindFile,
    CannotOpenFile,
    BadImage,
    Success
  };

  static auto Load(
    std::unique_ptr<CoreBase>& core,
    fs::path const& path
  ) -> Result;
};

} // namespace nba
