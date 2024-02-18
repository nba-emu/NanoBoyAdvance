/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/rom/backup/backup.hpp>
#include <nba/rom/backup/backup_file.hpp>
#include <nba/scheduler.hpp>
#include <string>

namespace nba {

struct EEPROM : Backup {
  enum Size {
    SIZE_4K  = 0,
    SIZE_64K = 1,
    DETECT = 2
  };
  
  EEPROM(fs::path const& save_path, Size size_hint, core::Scheduler& scheduler);
  
  void Reset() final;
  auto Read (u32 address) -> u8 final;
  void Write(u32 address, u8 value) final;
  
  void LoadState(SaveState const& state) final;
  void CopyState(SaveState& state) final;

  void SetSizeHint(Size size);

private:
  enum State {
    STATE_ACCEPT_COMMAND = 1 << 0,
    STATE_READ_MODE      = 1 << 1,
    STATE_WRITE_MODE     = 1 << 2,
    STATE_GET_ADDRESS    = 1 << 3,
    STATE_READING        = 1 << 4,
    STATE_DUMMY_NIBBLE   = 1 << 5,
    STATE_WRITING        = 1 << 6,
    STATE_EAT_DUMMY      = 1 << 7,
    STATE_BUSY           = 1 << 8
  };
  
  void ResetSerialBuffer();

  void OnReadyAfterWrite();

  int size;
  fs::path save_path;
  std::unique_ptr<BackupFile> file;

  core::Scheduler& scheduler;

  int state;
  int address;
  u64 serial_buffer;
  int transmitted_bits;

  bool detect_size;
};

} // namespace nba
