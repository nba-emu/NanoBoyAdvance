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

struct SRAM : Backup {
  SRAM(fs::path const& save_path);

  void Reset() final;  
  auto Read (u32 address) -> u8 final;
  void Write(u32 address, u8 value) final;
  
  void LoadState(SaveState const& state) final;
  void CopyState(SaveState& state) final;

private:
  fs::path save_path;
  std::unique_ptr<BackupFile> file;
};

} // namespace nba
