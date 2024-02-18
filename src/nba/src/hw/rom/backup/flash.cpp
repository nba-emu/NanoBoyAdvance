/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/rom/backup/flash.hpp>

namespace nba {

static constexpr int g_save_size[2] = { 65536, 131072 };

FLASH::FLASH(fs::path const& save_path, Size size_hint)
    : size(size_hint)
    , save_path(save_path) {
  Reset();
}
  
void FLASH::Reset() {
  current_bank = 0;
  phase = 0;
  enable_chip_id = false;
  enable_erase = false;
  enable_write = false;
  enable_select = false;
  
  int bytes = g_save_size[size];
  
  file = BackupFile::OpenOrCreate(save_path, { 65536, 131072 }, bytes);
  if(bytes == g_save_size[0]) {
    size = SIZE_64K;
  } else {
    size = SIZE_128K;
  }
}

auto FLASH::Read (u32 address) -> u8 {
  address &= 0xFFFF;
  
  // TODO: check if the Chip ID should be mirrored each 0x100 bytes.
  if(enable_chip_id && address < 2) {
    // Chip identifier for FLASH64: D4BF (SST 64K)
    // Chip identifier for FLASH128: 09C2 (Macronix 128K)
    if(size == SIZE_128K) {
      return (address == 0) ? 0xC2 : 0x09;
    } else {
      return (address == 0) ? 0xBF : 0xD4;
    }
  }
  
  return file->Read(Physical(address));
}

void FLASH::Write(u32 address, u8 value) {
  /* TODO: figure out how malformed sequences behave. */
  switch(phase) {
    case 0: {
      if(address == 0x0E005555 && value == 0xAA) {
        phase = 1;
      }
      break;
    }
    case 1: {
      if(address == 0x0E002AAA && value == 0x55) {
        phase = 2;
      }
      break;
    }
    case 2: {
      HandleCommand(address, value);
      break;
    }
    case 3: {
      HandleExtended(address, value);
      break;
    }
  }
}

void FLASH::HandleCommand(u32 address, u8 value) {
  if(address == 0x0E005555) {
    switch(static_cast<Command>(value)) {
      case READ_CHIP_ID: {
        enable_chip_id = true;
        phase = 0;
        break;
      }
      case FINISH_CHIP_ID: {
        enable_chip_id = false;
        phase = 0;
        break;
      }
      case ERASE: {
        enable_erase = true;
        phase = 0;
        break;
      }
      case ERASE_CHIP: {
        if(enable_erase) {
          file->MemorySet(0, g_save_size[size], 0xFF);
          enable_erase = false;
        }
        phase = 0;
        break;
      }
      case WRITE_BYTE: {
        enable_write = true;
        phase = 3;
        break;
      }
      case SELECT_BANK: {
        if(size == SIZE_128K) {
          enable_select = true;
          phase = 3;
        } else {
          phase = 0;
        }
        break;
      }
    }
  } else if(enable_erase && (address & ~0xF000) == 0x0E000000 && static_cast<Command>(value) == ERASE_SECTOR) {
    u32 base = address & 0xF000;
    
    file->MemorySet(Physical(base), 0x1000, 0xFF);
    enable_erase = false;
    phase = 0;
  }
}

void FLASH::HandleExtended(u32 address, u8 value) {
  if(enable_write) {
    file->Write(Physical(address & 0xFFFF), value);
    enable_write = false;
  } else if(enable_select && address == 0x0E000000) {
    current_bank = value & 1;
    enable_select = false;
  }
      
  phase = 0;
}

} // namespace nba
