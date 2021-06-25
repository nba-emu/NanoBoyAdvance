/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <common/integer.hpp>

namespace nba::core::arm {

struct MemoryBase {
  enum class Access {
    Nonsequential = 0,
    Sequential  = 1
  };

  virtual ~MemoryBase() = default;

  virtual u8  ReadByte(u32 address, Access access) = 0;
  virtual u16 ReadHalf(u32 address, Access access) = 0;
  virtual u32 ReadWord(u32 address, Access access) = 0;

  virtual void WriteByte(u32 address, u8  value, Access access) = 0;
  virtual void WriteHalf(u32 address, u16 value, Access access) = 0;
  virtual void WriteWord(u32 address, u32 value, Access access) = 0;

  virtual void Idle() = 0;
};

} // namespace nba::core::arm
