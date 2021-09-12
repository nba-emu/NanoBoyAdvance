/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/rom/header.hpp>
#include <nba/rom/rom.hpp>
#include <nba/rom/backup/eeprom.hpp>
#include <nba/rom/backup/flash.hpp>
#include <nba/rom/backup/sram.hpp>
#include <nba/log.hpp>
#include <platform/emulator.hpp>

#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <utility>
#include <string_view>

#include "game_db.hpp"

namespace nba {

namespace fs = std::filesystem;

using namespace nba::core;

using BackupType = Config::BackupType;

constexpr int g_bios_size = 0x4000;
constexpr int g_max_rom_size = 33554432; // 32 MiB

Emulator::Emulator(std::shared_ptr<Config> config)
    : config(config) {
  core = CreateCore(config);
  Reset();
}

Emulator::~Emulator() {
}

void Emulator::Reset() { core->Reset(); }

auto Emulator::DetectBackupType(u8* rom, size_t size) -> BackupType {
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
        Log<Info>("Emulator: ROM indicates '{}' backup memory.", std::to_string(type));
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
      throw nullptr;
  }
}

auto Emulator::CalculateMirrorMask(size_t size) -> u32 {
  u32 mask = 1;
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

  std::vector<u8> bios;
  bios.resize(size);
  stream.read((char*)bios.data(), size);
  stream.close();

  core->Attach(bios);

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
    return StatusCode::GameNotFound;
  }

  size = fs::file_size(path);

  if (size < sizeof(Header) || size > g_max_rom_size) {
    return StatusCode::GameWrongSize;
  }

  std::ifstream stream { path, std::ios::binary };

  /* TODO: most likely this error would only happen
   * if the file cannot be opened due to missing privileges.
   * The status code "Game not found" is not accurate, really.
   */
  if (!stream.good()) {
    return StatusCode::GameNotFound;
  }

  auto rom = std::vector<u8>{};
  rom.assign(std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>());
  stream.close();

  auto header = reinterpret_cast<Header*>(rom.data());
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
      game_info = match->second;
    }

    /* If not database entry was found or the entry has no backup type information,
     * try to search the ROM for strings that can indicate the
     * backup type used by this game.
     */
    if (game_info.backup_type == Config::BackupType::Detect) {
      game_info.backup_type = DetectBackupType(rom.data(), size);
      if (game_info.backup_type == Config::BackupType::Detect) {
        game_info.backup_type = Config::BackupType::SRAM;
        Log<Warn>("Emulator: failed to get backup type, using SRAM.");
      }
    }
  } else {
    game_info.backup_type = config->backup_type;
  }

  fmt::print("\tgame title: {}\n", game_title);
  fmt::print("\tgame code : {}\n", game_code);
  fmt::print("\tgame maker: {}\n", game_maker);
  fmt::print("\tmemory: {}\n", std::to_string(game_info.backup_type));
  fmt::print("\trtc:    {}\n", game_info.gpio == GPIODeviceType::RTC);
  fmt::print("\tROM mirrored: {}\n", game_info.mirror);

  // TODO: CreateBackupInstance should return a unique_ptr directly.
  auto backup = std::unique_ptr<Backup>{CreateBackupInstance(game_info.backup_type, save_path)};
  auto gpio = std::unique_ptr<GPIO>{};

  if (game_info.gpio == GPIODeviceType::RTC || config->force_rtc) {
    gpio = core->CreateRTC();
  }

  u32 mask = 0x01FF'FFFF;
  if (game_info.mirror) {
    mask = CalculateMirrorMask(size);
  }

  core->Attach(ROM{
    std::move(rom),
    std::move(backup),
    std::move(gpio),
    mask
  });

  return StatusCode::Ok;
}

void Emulator::Run(int cycles) {
  core->Run(cycles);
}

void Emulator::Frame() {
  core->RunForOneFrame();
}

} // namespace nba
