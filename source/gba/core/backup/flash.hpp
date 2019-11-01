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

class FLASH : public Backup {

public:
  enum Size {
    SIZE_64K  = 0,
    SIZE_128K = 1
  };
  
  FLASH(std::string const& save_path, Size size_hint);
 ~FLASH() final;
  
  void Reset() final;
  auto Read (std::uint32_t address) -> std::uint8_t final;
  void Write(std::uint32_t address, std::uint8_t value) final;

private:
  
  enum Command {
    READ_CHIP_ID = 0x90,
    FINISH_CHIP_ID = 0xF0,
    ERASE = 0x80,
    ERASE_CHIP = 0x10,
    ERASE_SECTOR = 0x30,
    WRITE_BYTE = 0xA0,
    SELECT_BANK = 0xB0
  };
  
  void HandleCommand(std::uint32_t address, std::uint8_t value);
  void HandleExtended(std::uint32_t address, std::uint8_t value);
  
  Size size;
  std::string save_path;
  
  std::uint8_t memory[2][65536];
  int current_bank;
  int phase;
  bool enable_chip_id;
  bool enable_erase;
  bool enable_write;
  bool enable_select;
};

}