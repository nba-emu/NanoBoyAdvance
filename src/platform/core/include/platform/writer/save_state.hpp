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

struct SaveStateWriter {
  enum class Result {
    CannotOpenFile,
    CannotWrite,
    Success
  };

  static auto Write(
    std::unique_ptr<CoreBase>& core,
    fs::path const& path
  ) -> Result;
};

} // namespace nba
