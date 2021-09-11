/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/backup/backup.hpp>
#include <nba/backup/backup_file.hpp>
#include <string>

namespace nba {

struct SRAM : Backup {
  SRAM(std::string const& save_path);

  void Reset() final;  
  auto Read (u32 address) -> u8 final;
  void Write(u32 address, u8 value) final;
  
private:
  std::string save_path;
  std::unique_ptr<BackupFile> file;
};

} // namespace nba
