/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#ifndef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif

#include <cartridge/header.hpp>
#include <cartridge/game_db.hpp>
#include <cartridge/backup/eeprom.hpp>
#include <cartridge/backup/flash.hpp>
#include <cartridge/backup/sram.hpp>
#include <common/log.hpp>
#include <cstring>
#include <exception>
#include <experimental/filesystem>
#include <fstream>
#include <map>

#include "emulator.hpp"

namespace nba {

namespace fs = std::experimental::filesystem;

using namespace nba::core;

using BackupType = Config::BackupType;

constexpr int g_cycles_per_frame = 280896;
constexpr int g_bios_size = 0x4000;
constexpr int g_max_rom_size = 33554432; // 32 MiB

Emulator::Emulator(std::shared_ptr<Config> config)
  : cpu(config)
  , config(config)
{
  Reset();
}

void Emulator::Reset() { cpu.Reset(); }

auto Emulator::DetectBackupType(std::uint8_t* rom, size_t size) -> BackupType {
  const std::map<std::string, Config::BackupType> signatures {
    { "EEPROM_V",   BackupType::EEPROM_64 },
    { "SRAM_V",     BackupType::SRAM      },
    { "FLASH_V",    BackupType::FLASH_64  },
    { "FLASH512_V", BackupType::FLASH_64  },
    { "FLASH1M_V",  BackupType::FLASH_128 }
  };
  
  for (int i = 0; i < size; i += 4) {
    for (auto const& pair : signatures) {
      auto const& string = pair.first;
      
      if (std::memcmp(&rom[i], string.c_str(), string.size()) == 0) {
        return pair.second;
      }
    }
  }
  
  return Config::BackupType::Detect;
}

auto Emulator::CreateBackupInstance(Config::BackupType backup_type, std::string save_path) -> Backup* {
  switch (backup_type) {
    case BackupType::SRAM:
    case BackupType::Detect: {
      return new SRAM(save_path);
    }
    case BackupType::FLASH_64: {
      return new FLASH(save_path, FLASH::SIZE_64K);
    }
    case BackupType::FLASH_128: {
      return new FLASH(save_path, FLASH::SIZE_128K);
    }
    case BackupType::EEPROM_4: {
      return new EEPROM(save_path, EEPROM::SIZE_4K);
    }
    case BackupType::EEPROM_64: {
      return new EEPROM(save_path, EEPROM::SIZE_64K);
    }
  }
}

auto Emulator::CalculateMirrorMask(size_t size) -> std::uint32_t {
  std::uint32_t mask = 1;
  while (mask < size) {
    mask *= 2;
  }
  return mask - 1;
}

auto Emulator::LoadBIOS() -> StatusCode {
  auto bios_path = config->bios_path;
  
  if (!fs::exists(bios_path) || !fs::is_regular_file(bios_path)) {
    return StatusCode::BiosNotFound;
  }
  
  auto size = fs::file_size(bios_path);
  
  if (size != g_bios_size) {
    return StatusCode::BiosWrongSize;
  }
  
  std::ifstream stream { bios_path, std::ios::binary };
  
  /* TODO: most likely this error would only happen
   * if the file cannot be opened due to missing privileges.
   * The status code "BIOS not found" is not accurate, really.
   */
  if (!stream.good()) {
    return StatusCode::BiosNotFound;
  }
  
  stream.read((char*)cpu.memory.bios, g_bios_size);
  stream.close();
  
  return StatusCode::Ok;
}

auto Emulator::LoadGame(std::string const& path) -> StatusCode {
  size_t size;
  
  /* If the BIOS was not loaded yet, load it now. */
  if (!bios_loaded) {
    auto status = LoadBIOS();
    if (status != StatusCode::Ok) {
      return status;
    }
    bios_loaded = true;
  }
  
  LOG_INFO("Loading {0}", path);

  /* Ensure ROM exists and has valid size. */
  if (!fs::exists(path) || !fs::is_regular_file(path)) {
    LOG_ERROR("The ROM path does not exist or does not point to a file.");
    return StatusCode::GameNotFound;
  }
  size = fs::file_size(path);
  if (size < sizeof(Header)) {
    LOG_ERROR("ROM size is too low, no valid header.");
    return StatusCode::GameWrongSize;
  }
  if (size > g_max_rom_size) {
    LOG_ERROR("ROM size exceeds maximum size of 32 MiB.");
    return StatusCode::GameWrongSize;
  }
  
  std::ifstream stream { path, std::ios::binary };
  
  /* TODO: most likely this error would only happen
   * if the file cannot be opened due to missing privileges.
   * The status code "Game not found" is not accurate, really.
   */
  if (!stream.good()) {
    LOG_ERROR("Failed to open ROM file with unknown error.");
    return StatusCode::GameNotFound;
  }
    
  auto rom = std::make_unique<std::uint8_t[]>(size);
  Header* header = reinterpret_cast<Header*>(rom.get());
  stream.read((char*)(rom.get()), size);
  stream.close();
  
  std::string save_path = path.substr(0, path.find_last_of(".")) + ".sav";

  GameInfo game_info;

  std::string game_title;
  std::string game_code;
  std::string game_maker;

  game_title.assign(header->game.title, 12);
  game_code.assign(header->game.code, 4);
  game_maker.assign(header->game.maker, 2);

  /* If no save type was specified try to determine it from a list of
   * game override. Alternatively if that doesn't work, look for some
   * Nintendo SDK strings, that can reveal the save type.
   */
  if (config->backup_type == Config::BackupType::Detect) {
    /* Try to match gamecode with game database. */
    auto match = g_game_db.find(game_code);
    if (match != g_game_db.end()) {
      LOG_INFO("Successfully matched ROM to game database entry.");
      game_info = match->second;
    }

    if (game_info.backup_type == Config::BackupType::Detect) {
      LOG_INFO("Could not determine backup type from game database. Searching for SDK pattern...");
      game_info.backup_type = DetectBackupType(rom.get(), size);
      if (game_info.backup_type == Config::BackupType::Detect) {
        game_info.backup_type = Config::BackupType::SRAM;
        LOG_WARN("Failed to determine backup type, fallback to SRAM.");
      }
    }
  } else {
    game_info.backup_type = config->backup_type;
  }

  LOG_INFO("Game Title: {0}", game_title);
  LOG_INFO("Game Code:  {0}", game_code);
  LOG_INFO("Game Maker: {0}", game_maker);
  LOG_INFO("Backup: {0}", std::to_string(game_info.backup_type));
  LOG_INFO("RTC:    {0}", game_info.gpio == GPIO::RTC);
  LOG_INFO("Mirror: {0}", game_info.mirror);
  
  /* Mount cartridge into the cartridge slot ;-) */
  cpu.memory.rom.data = std::move(rom);
  cpu.memory.rom.size = size;
  cpu.memory.rom.backup_type = game_info.backup_type;
  cpu.memory.rom.backup = std::unique_ptr<Backup>{ CreateBackupInstance(game_info.backup_type, save_path) };

  /* Some games (e.g. Classic NES series) have the ROM memory mirrored. */
  if (game_info.mirror) {
    cpu.memory.rom.mask = CalculateMirrorMask(size);
  } else {
    cpu.memory.rom.mask = 0x1FFFFFF;
  }

  return StatusCode::Ok;
}

void Emulator::Frame() {
  cpu.RunFor(g_cycles_per_frame);
}

} // namespace nba
