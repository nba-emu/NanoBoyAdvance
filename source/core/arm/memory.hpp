/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

namespace ARM {

class MemoryBase {
public:
  enum class Access {
    Nonsequential = 0,
    Sequential  = 1
  };

  virtual std::uint8_t  ReadByte(std::uint32_t address, Access access) = 0;
  virtual std::uint16_t ReadHalf(std::uint32_t address, Access access) = 0;
  virtual std::uint32_t ReadWord(std::uint32_t address, Access access) = 0;

  virtual void WriteByte(std::uint32_t address, std::uint8_t  value, Access access) = 0;
  virtual void WriteHalf(std::uint32_t address, std::uint16_t value, Access access) = 0;
  virtual void WriteWord(std::uint32_t address, std::uint32_t value, Access access) = 0;

  virtual void Idle() = 0;
};

}
