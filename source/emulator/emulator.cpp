/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#ifndef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif

#include <emulator/cartridge/header.hpp>
#include <emulator/cartridge/game_db.hpp>
#include <emulator/cartridge/backup/eeprom.hpp>
#include <emulator/cartridge/backup/flash.hpp>
#include <emulator/cartridge/backup/sram.hpp>
#include <emulator/cartridge/gpio/rtc.hpp>
#include <common/log.hpp>
#include <cstring>
#include <exception>
#include <experimental/filesystem>
#include <fstream>
#include <utility>
#include <string_view>

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
  static constexpr std::pair<std::string_view, Config::BackupType> signatures[6] {
    { "EEPROM_V",   BackupType::EEPROM_64 },
    { "SRAM_V",     BackupType::SRAM      },
    { "SRAM_F_V",   BackupType::SRAM      },
    { "FLASH_V",    BackupType::FLASH_64  },
    { "FLASH512_V", BackupType::FLASH_64  },
    { "FLASH1M_V",  BackupType::FLASH_128 }
  };

  for (int i = 0; i < size; i += 4) {
    for (auto const& [signature, type] : signatures) {
      if ((i + signature.size()) <= size && std::memcmp(&rom[i], signature.data(), signature.size()) == 0) {
        LOG_INFO("Found ROM string indicating {0} backup type.", std::to_string(type));
        return type;
      }
    }
  }

  return Config::BackupType::Detect;
}

auto Emulator::CreateBackupInstance(Config::BackupType backup_type, std::string save_path) -> Backup* {
  switch (backup_type) {
    case BackupType::SRAM:
      return new SRAM(save_path);
    case BackupType::FLASH_64:
      return new FLASH(save_path, FLASH::SIZE_64K);
    case BackupType::FLASH_128:
      return new FLASH(save_path, FLASH::SIZE_128K);
    case BackupType::EEPROM_4:
      return new EEPROM(save_path, EEPROM::SIZE_4K);
    case BackupType::EEPROM_64:
      return new EEPROM(save_path, EEPROM::SIZE_64K);
    default:
      throw std::logic_error("CreateBackupInstance: bad backup type 'Detect'.");
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
    LOG_ERROR("Unable to load BIOS file: {0}", bios_path);
    return StatusCode::BiosNotFound;
  }

  auto size = fs::file_size(bios_path);

  if (size != g_bios_size) {
    LOG_ERROR("BIOS file has unexpected size, expected file of {0} bytes.", g_bios_size);
    return StatusCode::BiosWrongSize;
  }

  std::ifstream stream { bios_path, std::ios::binary };

  /* TODO: most likely this error would only happen
   * if the file cannot be opened due to missing privileges.
   * The status code "BIOS not found" is not accurate, really.
   */
  if (!stream.good()) {
    LOG_ERROR("Failed to open BIOS file with unknown error.");
    return StatusCode::BiosNotFound;
  }

  stream.read((char*)cpu.memory.bios, g_bios_size);
  stream.close();

  return StatusCode::Ok;
}

auto Emulator::LoadGame(std::string const& path) -> StatusCode {
  size_t size;
  GameInfo game_info;
  std::string game_title;
  std::string game_code;
  std::string game_maker;
  std::string save_path = path.substr(0, path.find_last_of(".")) + ".sav";

  /* If the BIOS was not loaded yet, load it now. */
  if (!bios_loaded) {
    auto status = LoadBIOS();
    if (status != StatusCode::Ok) {
      return status;
    }
    bios_loaded = true;
  }

  /* Ensure ROM exists and has valid size. */
  if (!fs::exists(path) || !fs::is_regular_file(path)) {
    LOG_ERROR("The ROM path does not exist or does not point to a file.");
    return StatusCode::GameNotFound;
  }

  size = fs::file_size(path);

  if (size < sizeof(Header) || size > g_max_rom_size) {
    LOG_ERROR("ROM file has unexpected size, expected file size between {0} bytes and 32 MiB.", sizeof(Header));
    return StatusCode::GameWrongSize;
  }

  std::ifstream stream { path, std::ios::binary };

  /* TODO: most likely this error would only happen
   * if the file cannot be opened due to missing privileges.
   * The status code "Game not found" is not accurate, really.
   */
  if (!stream.good()) {
    LOG_ERROR("Failed to open ROM with unknown error.");
    return StatusCode::GameNotFound;
  }

  auto rom = std::make_unique<std::uint8_t[]>(size);
  Header* header = reinterpret_cast<Header*>(rom.get());
  stream.read((char*)(rom.get()), size);
  stream.close();

  game_title.assign(header->game.title, 12);
  game_code.assign(header->game.code, 4);
  game_maker.assign(header->game.maker, 2);

  /* If no save type was specified try to determine it from a list of
   * game override. Alternatively if that doesn't work, look for some
   * Nintendo SDK strings, that can reveal the save type.
   */
  if (config->backup_type == Config::BackupType::Detect) {
    /* Try to match gamecode with game database. */
    if (auto match = g_game_db.find(game_code); match != g_game_db.end()) {
      LOG_INFO("Successfully matched ROM to game database entry.");
      game_info = match->second;
    }

    /* If not database entry was found or the entry has no backup type information,
     * try to search the ROM for strings that can indicate the
     * backup type used by this game.
     */
    if (game_info.backup_type == Config::BackupType::Detect) {
      LOG_INFO("Unable to get backup type from game database.");
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
  LOG_INFO("RTC:    {0}", game_info.gpio == GPIODeviceType::RTC);
  LOG_INFO("Mirror: {0}", game_info.mirror);

  /* Mount cartridge into the cartridge slot. */
  cpu.memory.rom.data = std::move(rom);
  cpu.memory.rom.size = size;
  if (game_info.backup_type == Config::BackupType::EEPROM_4 || game_info.backup_type == Config::BackupType::EEPROM_64) {
    cpu.memory.rom.backup_sram.reset();
    cpu.memory.rom.backup_eeprom = std::unique_ptr<Backup>{ CreateBackupInstance(game_info.backup_type, save_path) };
  } else if (game_info.backup_type != Config::BackupType::None) {
    cpu.memory.rom.backup_sram = std::unique_ptr<Backup>{ CreateBackupInstance(game_info.backup_type, save_path) };
    cpu.memory.rom.backup_eeprom.reset();
  }

  /* Create and mount RTC if necessary. */
  if (game_info.gpio == GPIODeviceType::RTC || config->force_rtc) {
    cpu.memory.rom.gpio = std::make_unique<RTC>(&cpu.scheduler, &cpu.irq_controller);
  } else {
    cpu.memory.rom.gpio.reset();
  }

  /* Some games (e.g. Classic NES series) have the ROM memory mirrored. */
  if (game_info.mirror) {
    cpu.memory.rom.mask = CalculateMirrorMask(size);
  } else {
    cpu.memory.rom.mask = 0x1FFFFFF;
  }

  return StatusCode::Ok;
}

void Emulator::Run(int cycles) {
  cpu.RunFor(cycles);
}

void Emulator::Frame() {
  cpu.RunFor(g_cycles_per_frame);
}

} // namespace nba
