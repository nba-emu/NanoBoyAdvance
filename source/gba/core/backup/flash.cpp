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

#include <cstring>
#include <experimental/filesystem>
#include <fstream>


#include <cstdio>

#include "flash.hpp"

using namespace GameBoyAdvance;

namespace fs = std::experimental::filesystem;

static constexpr int g_save_size[2] = { 65536, 131072 };

FLASH::FLASH(std::string const& save_path, Size size_hint)
  : save_path(save_path)
  , size(size_hint)
{
  Reset();
}

FLASH::~FLASH() {
  
}
  
void FLASH::Reset() {
  current_bank = 0;
  phase = 0;
  enable_chip_id = false;
  enable_erase = false;
  enable_write = false;
  enable_select = false;
  
  //if (fs::exists(save_path)) {
  if (false) {
    auto save_size = fs::file_size(save_path);
    
    /* ... */
  } else {
    std::memset(memory, sizeof(memory), 0xFF);
  }
}

auto FLASH::Read (std::uint32_t address) -> std::uint8_t {
  address &= 0xFFFF;
  
  /* TODO(accuracy): check if the Chip ID is mirrored each 0x100 bytes. */
  if (enable_chip_id && address < 2) {
    // Chip identifier for FLASH64: D4BF (SST 64K)
    // Chip identifier for FLASH128: 09C2 (Macronix 128K)
    if (size == SIZE_128K) {
      return (address == 0) ? 0xC2 : 0x09;
    } else {
      return (address == 0) ? 0xBF : 0xD4;
    }
  }
  
  return memory[current_bank][address];
}

void FLASH::Write(std::uint32_t address, std::uint8_t value) {
  if (enable_write) {
    memory[current_bank][address & 0xFFFF] = value;
    enable_write = false;
    return;
  }
  
  /* TODO: what happens with malformed writes? */
  
  if (address == 0x0E005555 && value == 0xAA) {
    phase = 1;
    return;
  }
  
  if (address == 0x0E002AAA && value == 0x55 && phase == 1) {
    phase = 2;
    return;
  }
  
  if (address == 0x0E005555 && phase == 2) {
    switch (static_cast<Command>(value)) {
      case READ_CHIP_ID: {
        enable_chip_id = true;
        return;
      }
      case FINISH_CHIP_ID: {
        enable_chip_id = false;
        return;
      }
      case ERASE: {
        enable_erase = true;
        return;
      }
      case ERASE_CHIP: {
        if (enable_erase) {
          std::memset(memory, sizeof(memory), 0xFF);
          enable_erase = false;
        }
        return;
      }
      case WRITE_BYTE: {
        enable_write = true;
        return;
      }
      case SELECT_BANK: {
        /* TODO: maybe it would be better to break if the chip is 64K? */
        if (size == SIZE_128K) {
          enable_select = true;
        }
        return;
      }
      default: return;
    }
    
    phase = 0;
    return;
  }
  
  if (enable_erase && static_cast<Command>(value) == ERASE_SECTOR &&
      (address & ~0xF000) == 0x0E000000 && phase == 2) {
    std::uint32_t base = address & 0xF000;
    
    for (int i = 0; i < 0x1000; i++) {
      memory[current_bank][base + i] = 0xFF;
    }
    
    enable_erase = false;
    phase = 0;
    return;
  }
  
  if (enable_select && address == 0x0E000000) {
    current_bank = value & 1;
    enable_select = false;
    /* Why do we not set phase = 0? */
    // return;
  }
}