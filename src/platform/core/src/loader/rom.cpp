/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <filesystem>
#include <fstream>
#include <platform/loader/rom.hpp>
#include <nba/rom/backup/eeprom.hpp>
#include <nba/rom/backup/flash.hpp>
#include <nba/rom/backup/sram.hpp>
#include <nba/rom/header.hpp>
#include <nba/rom/rom.hpp>
#include <nba/log.hpp>
#include <string_view>
#include <utility>

namespace fs = std::filesystem;

namespace nba {

using BackupType = Config::BackupType;

static constexpr size_t kMaxROMSize = 32 * 1024 * 1024; // 32 MiB

auto ROMLoader::Load(
  std::unique_ptr<CoreBase>& core,
  std::string path,
  Config::BackupType backup_type,
  bool force_rtc
) -> Result {
  auto save_path = path.substr(0, path.find_last_of(".")) + ".sav";

  return Load(core, path, save_path, backup_type, force_rtc);
}

auto ROMLoader::Load(
  std::unique_ptr<CoreBase>& core,
  std::string rom_path,
  std::string save_path,
  BackupType backup_type,
  bool force_rtc
) -> Result {
  if (!fs::exists(rom_path)) {
    return Result::CannotFindFile;
  }

  if (fs::is_directory(rom_path)) {
    return Result::CannotOpenFile;
  }

  auto size = fs::file_size(rom_path);
  if (size < sizeof(Header) || size > kMaxROMSize) {
    return Result::BadImage;
  }

  auto file_stream = std::ifstream{rom_path, std::ios::binary};
  auto file_data = std::vector<u8>{};
  if (!file_stream.good()) {
    return Result::CannotOpenFile;
  }
  file_data.resize(size);
  file_stream.read((char*)file_data.data(), size);

  auto game_info = GetGameInfo(file_data);

  if (backup_type == BackupType::Detect) {
    if (game_info.backup_type != BackupType::Detect) {
      backup_type = game_info.backup_type;
    } else {
      backup_type = GetBackupType(file_data);
      if (backup_type == BackupType::Detect) {
        Log<Warn>("ROMLoader: failed to detect backup type!");
        backup_type = BackupType::SRAM;
      }
    }
  }

  auto backup = CreateBackup(save_path, backup_type);

  auto gpio = std::unique_ptr<GPIO>{};
  if (game_info.gpio == GPIODeviceType::RTC || force_rtc) {
    gpio = core->CreateRTC();
  }

  u32 rom_mask = u32(kMaxROMSize - 1);
  if (game_info.mirror) {
    rom_mask = u32(RoundSizeToPowerOfTwo(size) - 1);
  }

  core->Attach(ROM{
    std::move(file_data),
    std::move(backup),
    std::move(gpio),
    rom_mask
  });
  return Result::Success;
}

auto ROMLoader::GetGameInfo(
  std::vector<u8>& file_data
) -> GameInfo {
  auto header = reinterpret_cast<Header*>(file_data.data());
  auto game_code = std::string{};
  game_code.assign(header->game.code, 4);

  auto db_entry = g_game_db.find(game_code);
  if (db_entry != g_game_db.end()) {
    return db_entry->second;
  }

  return GameInfo{};
}

auto ROMLoader::GetBackupType(
  std::vector<u8>& file_data
) -> BackupType {
  static constexpr std::pair<std::string_view, BackupType> signatures[6] {
    { "EEPROM_V",   BackupType::EEPROM_64 },
    { "SRAM_V",     BackupType::SRAM      },
    { "SRAM_F_V",   BackupType::SRAM      },
    { "FLASH_V",    BackupType::FLASH_64  },
    { "FLASH512_V", BackupType::FLASH_64  },
    { "FLASH1M_V",  BackupType::FLASH_128 }
  };

  const auto size = file_data.size();

  for (int i = 0; i < size; i += sizeof(u32)) {
    for (auto const& [signature, type] : signatures) {
      if ((i + signature.size()) <= size &&
          std::memcmp(&file_data[i], signature.data(), signature.size()) == 0) {
        return type;
      }
    }
  }

  return BackupType::Detect;
}

auto ROMLoader::CreateBackup(
  std::string save_path,
  BackupType backup_type
) -> std::unique_ptr<Backup> {
  switch (backup_type) {
    case BackupType::SRAM: {
      return std::make_unique<SRAM>(save_path);
    }
    case BackupType::FLASH_64: {
      return std::make_unique<FLASH>(save_path, FLASH::SIZE_64K);
    }
    case BackupType::FLASH_128: {
      return std::make_unique<FLASH>(save_path, FLASH::SIZE_128K);
    }
    case BackupType::EEPROM_4: {
      return std::make_unique<EEPROM>(save_path, EEPROM::SIZE_4K);
    }
    case BackupType::EEPROM_64: {
      return std::make_unique<EEPROM>(save_path, EEPROM::SIZE_64K);
    }
  }

  return {};
}

auto ROMLoader::RoundSizeToPowerOfTwo(size_t size) -> size_t {
  size_t pot_size = 1;

  while (pot_size < size) {
    pot_size *= 2;
  }

  return pot_size;
}

} // namespace nba
