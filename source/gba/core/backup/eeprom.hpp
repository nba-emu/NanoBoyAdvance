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

#include <string>

#include "backup.hpp"

namespace GameBoyAdvance {

class EEPROM : public Backup {

public:
  enum Size {
    SIZE_4K  = 0,
    SIZE_64K = 1
  };
  
  EEPROM(std::string const& save_path, Size size_hint);
  
  void Reset() final;
  auto Read (std::uint32_t address) -> std::uint8_t final;
  void Write(std::uint32_t address, std::uint8_t value) final;
  
private:
  enum State {
    STATE_ACCEPT_COMMAND = 1 << 0,
    STATE_READ_MODE      = 1 << 1,
    STATE_WRITE_MODE     = 1 << 2,
    STATE_GET_ADDRESS    = 1 << 3,
    STATE_READING        = 1 << 4,
    STATE_DUMMY_NIBBLE   = 1 << 5,
    STATE_WRITING        = 1 << 6,
    STATE_EAT_DUMMY      = 1 << 7
  };
  
  void ResetSerialBuffer();
  
  int size;
  std::string save_path;

  /* TODO: use an unique_ptr here? */
  std::uint8_t* memory = nullptr;
  
  int state;
  int address; /* TODO: what is the appropriate data type? */
  std::uint64_t serial_buffer;
  int transmitted_bits;
};

}