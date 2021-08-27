/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/deprecate/config.hpp>
#include <memory>
#include <string>

namespace nba {

namespace core {
struct CPU;
} // namespace nba::core

struct Backup;

struct Emulator {
  enum class StatusCode {
    BiosNotFound,
    GameNotFound,
    BiosWrongSize,
    GameWrongSize,
    Ok
  };
  
  Emulator(std::shared_ptr<Config> config);
 ~Emulator();

  void Reset();
  auto LoadGame(std::string const& path) -> StatusCode;
  void Run(int cycles);
  void Frame();
  
private:
  static auto DetectBackupType(u8* rom, size_t size) -> Config::BackupType;
  static auto CreateBackupInstance(Config::BackupType backup_type, std::string save_path) -> Backup*;
  static auto CalculateMirrorMask(size_t size) -> u32;
  
  auto LoadBIOS() -> StatusCode; 
  
  core::CPU* cpu;
  bool bios_loaded = false;
  std::shared_ptr<Config> config;
};

} // namespace nba
