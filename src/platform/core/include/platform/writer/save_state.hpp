/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <string>

namespace nba {

struct SaveStateWriter {
  enum class Result {
    CannotOpenFile,
    CannotWrite,
    Success
  };

  static auto Write(
    std::unique_ptr<CoreBase>& core,
    std::string path
  ) -> Result;
};

} // namespace nba
