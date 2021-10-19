/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <nba/rom/backup/backup.hpp>
#include <platform/game_db.hpp>
#include <string>

namespace nba {

struct ROMLoader {
  enum class Result {
    CannotFindFile,
    CannotOpenFile,
    BadImage,
    Success
  };

  static auto Load(
    std::unique_ptr<CoreBase>& core,
    std::string path,
    Config::BackupType backup_type = Config::BackupType::Detect,
    bool force_rtc = true
  ) -> Result;

  static auto Load(
    std::unique_ptr<CoreBase>& core,
    std::string rom_path,
    std::string save_path,
    Config::BackupType backup_type = Config::BackupType::Detect,
    bool force_rtc = true
  ) -> Result;

private:
  static auto GetGameInfo(
    std::vector<u8>& file_data
  ) -> GameInfo;

  static auto GetBackupType(
    std::vector<u8>& file_data
  ) -> Config::BackupType;

  static auto CreateBackup(
    std::string save_path,
    Config::BackupType backup_type
  ) -> std::unique_ptr<Backup>;

  static auto RoundSizeToPowerOfTwo(size_t size) -> size_t;
};

} // namespace nba
