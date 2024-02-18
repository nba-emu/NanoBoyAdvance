/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/rom/backup/backup.hpp>
#include <nba/rom/backup/backup_file.hpp>
#include <string>

namespace nba {

struct FLASH : Backup {
  enum Size {
    SIZE_64K  = 0,
    SIZE_128K = 1
  };
  
  FLASH(fs::path const& save_path, Size size_hint);
  
  void Reset() final;
  auto Read (u32 address) -> u8 final;
  void Write(u32 address, u8 value) final;

  void LoadState(SaveState const& state) final;
  void CopyState(SaveState& state) final;

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
  
  void HandleCommand(u32 address, u8 value);
  void HandleExtended(u32 address, u8 value);
  
  auto Physical(int index) -> int { return current_bank * 65536 + index; }
  
  Size size;
  fs::path save_path;
  std::unique_ptr<BackupFile> file;
  
  int current_bank;
  int phase;
  bool enable_chip_id;
  bool enable_erase;
  bool enable_write;
  bool enable_select;
};

} // namespace nba
