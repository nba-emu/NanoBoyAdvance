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

class SRAM : public Backup {
public:
  SRAM(std::string const& save_path)
    : save_path(save_path) {
    Reset();
  }
  
  void Reset() final {
    int bytes = 32768;
    file = BackupFile::OpenOrCreate(save_path, { 32768 }, bytes);
  }
  
  auto Read(std::uint32_t address) -> std::uint8_t final {
    return file->Read(address & 0x7FFF);
  }
  
  void Write(std::uint32_t address, std::uint8_t value) final {
    file->Write(address & 0x7FFF, value);
  }
  
private:
  std::string save_path;
  std::unique_ptr<BackupFile> file;
};

} // namespace nba
