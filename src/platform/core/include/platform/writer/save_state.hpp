/*
 * Copyright (C) 2024 fleroviux
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
