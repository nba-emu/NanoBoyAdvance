// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/core.hh>
#include <filesystem>

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
