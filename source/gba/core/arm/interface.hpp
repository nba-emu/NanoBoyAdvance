/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
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
