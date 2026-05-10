// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

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
