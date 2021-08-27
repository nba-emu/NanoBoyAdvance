/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>

namespace nba { 

struct Backup {
  virtual ~Backup() = default;

  virtual void Reset() = 0;
  virtual auto Read (u32 address) -> u8 = 0;
  virtual void Write(u32 address, u8 value) = 0;
};

} // namespace nba
