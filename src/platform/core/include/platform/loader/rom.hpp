/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/core.hpp>
#include <nba/rom/backup/backup.hpp>
#include <platform/game_db.hpp>
#include <string>

namespace fs = std::filesystem;

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
    fs::path const& path,
    Config::BackupType backup_type = Config::BackupType::Detect,
    GPIODeviceType force_gpio = GPIODeviceType::None
  ) -> Result;

  static auto Load(
    std::unique_ptr<CoreBase>& core,
    fs::path const& rom_path,
    fs::path const& save_path,
    Config::BackupType backup_type = Config::BackupType::Detect,
    GPIODeviceType force_gpio = GPIODeviceType::None
  ) -> Result;

private:
  static auto ReadFile(fs::path const& path, std::vector<u8>& file_data) -> Result;
  static auto ReadFileFromArchive(fs::path const& path, std::vector<u8>& file_data) -> Result;

  static auto GetGameInfo(
    std::vector<u8>& file_data
  ) -> GameInfo;

  static auto GetBackupType(
    std::vector<u8>& file_data
  ) -> Config::BackupType;

  static auto CreateBackup(
    fs::path const& save_path,
    Config::BackupType backup_type
  ) -> std::unique_ptr<Backup>;

  static auto RoundSizeToPowerOfTwo(size_t size) -> size_t;
};

} // namespace nba
