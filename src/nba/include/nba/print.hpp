/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <fmt/format.h>
#include <stdexcept>

namespace nba {

  template<typename... Args>
  void print(Args&&... args) {
    // {fmt} throws an exception if it cannot write to stdout.
    // This unfortunately is the case when running as a windowed application with no console attached. 
    // See: https://github.com/fmtlib/fmt/issues/1176
    try {
      fmt::print(std::forward<Args>(args)...);
    } catch(const std::system_error& _) {
    }
  }

} // namespace nba
