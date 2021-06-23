/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <string>

#include "backup.hpp"
#include "backup_file.hpp"

namespace nba {

class FLASH : public Backup {
public:
  enum Size {
    SIZE_64K  = 0,
    SIZE_128K = 1
  };
  
  FLASH(std::string const& save_path, Size size_hint);
  
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
  
  auto Physical(int index) -> int { return current_bank * 65536 + index; }
  
  Size size;
  std::string save_path;
  std::unique_ptr<BackupFile> file;
  
  int current_bank;
  int phase;
  bool enable_chip_id;
  bool enable_erase;
  bool enable_write;
  bool enable_select;
};

} // namespace nba
