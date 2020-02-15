/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

namespace ARM {

enum AccessType {
  ACCESS_NSEQ = 0,
  ACCESS_SEQ  = 1,
};

struct Interface {
  virtual std::uint8_t  ReadByte(std::uint32_t address, AccessType type) = 0;
  virtual std::uint16_t ReadHalf(std::uint32_t address, AccessType type) = 0;
  virtual std::uint32_t ReadWord(std::uint32_t address, AccessType type) = 0;

  virtual void WriteByte(std::uint32_t address, std::uint8_t  value, AccessType type) = 0;
  virtual void WriteHalf(std::uint32_t address, std::uint16_t value, AccessType type) = 0;
  virtual void WriteWord(std::uint32_t address, std::uint32_t value, AccessType type) = 0;

  virtual void Tick(int cycles) = 0;
  virtual void Idle() = 0;
  virtual void SWI(std::uint32_t call_id) = 0;
};

}
