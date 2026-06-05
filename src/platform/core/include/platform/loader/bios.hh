// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/core.hh>
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

  static auto LoadEmbedded(std::unique_ptr<CoreBase>& core) -> Result;

  static auto Validate(fs::path const& path) -> Result;
  static auto Describe(Result result) -> const char*;
};

} // namespace nba
