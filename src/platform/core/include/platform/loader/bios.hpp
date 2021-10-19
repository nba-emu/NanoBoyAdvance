/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <string>

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
    std::string path
  ) -> Result;
};

} // namespace nba
