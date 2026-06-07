// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <nba/rom/backup/eeprom.hh>
#include <nba/rom/backup/flash.hh>
#include <nba/rom/backup/sram.hh>
#include <nba/rom/header.hh>
#include <nba/rom/rom.hh>
#include <platform/loader/rom.hh>
#include <atom/logger/logger.hh>
#include <unarr.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <cstddef>
#include <string_view>
#include <utility>

namespace nba {

using BackupType = Config::BackupType;

static constexpr size_t kMaxROMSize = 32 * 1024 * 1024; // 32 MiB
static constexpr size_t kHeaderChecksumStart = 0xA0;
static constexpr size_t kHeaderChecksumEnd = 0xBC;

#if defined(PLATFORM_DREAMCAST)
static constexpr bool kDreamcastForceFlatSmallROMs = false;
static constexpr size_t kDreamcastFlatROMLimit = 8 * 1024 * 1024;

static void DreamcastLoaderTrace(char const* phase, fs::path const& path, size_t size = 0) {
  std::printf("[NBA-DC] ROMLoader %s: %s", phase, path.string().c_str());
  if(size != 0) {
    std::printf(" (%lu bytes)", static_cast<unsigned long>(size));
  }
  std::printf("\n");
  std::fflush(stdout);
}

static auto IsDreamcastVirtualPath(fs::path const& path) -> bool {
  const auto path_string = path.string();
  return path_string.rfind("/cd/", 0) == 0 || path_string.rfind("/pc/", 0) == 0;
}

// Scans a ROM file in chunks to detect the save backup type from embedded
// signature strings (e.g. "SRAM_V", "EEPROM_V").  This avoids loading the
// entire cartridge into memory, which is essential for large ROMs on Dreamcast.
static auto GetBackupTypeFromFile(fs::path const& path, size_t rom_size) -> Config::BackupType {
  using BackupType = Config::BackupType;

  static constexpr std::pair<std::string_view, BackupType> kSignatures[] {
    { "EEPROM_V",   BackupType::EEPROM_DETECT },
    { "SRAM_V",     BackupType::SRAM },
    { "SRAM_F_V",   BackupType::SRAM },
    { "FLASH_V",    BackupType::FLASH_64 },
    { "FLASH512_V", BackupType::FLASH_64 },
    { "FLASH1M_V",  BackupType::FLASH_128 }
  };
  static constexpr size_t kMaxSigLen = 10; // length of longest possible signature
  static constexpr size_t kChunkSize = 64 * 1024;

  auto* file = std::fopen(path.string().c_str(), "rb");
  if(!file) {
    return BackupType::Detect;
  }

  // Allocate one extra kMaxSigLen bytes so signatures that straddle a chunk
  // boundary are still found when we scan up to kChunkSize positions per chunk.
  std::vector<u8> chunk(kChunkSize + kMaxSigLen, 0xFF);
  BackupType result = BackupType::Detect;
  size_t file_offset = 0;

  while(file_offset < rom_size && result == BackupType::Detect) {
    const size_t to_read = std::min(kChunkSize + kMaxSigLen, rom_size - file_offset);
    if(std::fseek(file, static_cast<long>(file_offset), SEEK_SET) != 0) break;
    const size_t bytes_read = std::fread(chunk.data(), 1, to_read, file);
    if(bytes_read == 0) break;

    // Scan at 4-byte aligned offsets (matching the in-memory scanner in
    // GetBackupType).  The extra kMaxSigLen bytes let us safely compare
    // signatures whose first character lands at the very end of the chunk.
    const size_t scan_limit = std::min(bytes_read, kChunkSize);
    for(size_t i = 0; i + sizeof(u32) <= scan_limit; i += sizeof(u32)) {
      for(auto const& [sig, type] : kSignatures) {
        if(i + sig.size() <= bytes_read &&
            std::memcmp(&chunk[i], sig.data(), sig.size()) == 0) {
          result = type;
          break;
        }
      }
      if(result != BackupType::Detect) break;
    }

    file_offset += kChunkSize;
  }

  std::fclose(file);
  return result;
}
#endif

struct ParsedROMHeader {
  std::string title;
  std::string code;
  std::string maker;
  u8 unit_code = 0;
  u8 device_type = 0;
  u8 version = 0;
  u8 checksum = 0;
  bool fixed_marker_ok = false;
  bool checksum_ok = false;
  GameInfo game_info;
};

static auto TrimHeaderString(char const* data, size_t size) -> std::string {
  auto result = std::string{data, data + size};
  while(!result.empty() && (result.back() == '\0' || result.back() == ' ')) {
    result.pop_back();
  }
  return result;
}

static auto HeaderChecksumMatches(std::vector<u8> const& file_data) -> bool {
  if(file_data.size() <= offsetof(Header, checksum)) {
    return false;
  }

  u8 checksum = 0;
  for(size_t i = kHeaderChecksumStart; i <= kHeaderChecksumEnd; i++) {
    checksum = static_cast<u8>(checksum - file_data[i]);
  }
  checksum = static_cast<u8>(checksum - 0x19);
  return checksum == file_data[offsetof(Header, checksum)];
}

static auto ParseROMHeader(std::vector<u8> const& file_data) -> ParsedROMHeader {
  auto info = ParsedROMHeader{};
  if(file_data.size() < sizeof(Header)) {
    return info;
  }

  auto const* header = reinterpret_cast<Header const*>(file_data.data());
  info.title = TrimHeaderString(header->game.title, sizeof(header->game.title));
  info.code = TrimHeaderString(header->game.code, sizeof(header->game.code));
  info.maker = TrimHeaderString(header->game.maker, sizeof(header->game.maker));
  info.unit_code = header->unit_code;
  info.device_type = header->device_type;
  info.version = header->version;
  info.checksum = header->checksum;
  info.fixed_marker_ok = header->fixed_96h == 0x96;
  info.checksum_ok = HeaderChecksumMatches(file_data);
  if(auto db_entry = g_game_db.find(info.code); db_entry != g_game_db.end()) {
    info.game_info = db_entry->second;
  }
  return info;
}

static void TraceROMHeader(ParsedROMHeader const& info, fs::path const& path) {
  std::printf(
    "[NBA-DC] ROM header: %s title='%s' code='%s' maker='%s' unit=0x%02X device=0x%02X version=0x%02X checksum=0x%02X fixed=%s checksum_ok=%s\n",
    path.string().c_str(),
    info.title.c_str(),
    info.code.c_str(),
    info.maker.c_str(),
    info.unit_code,
    info.device_type,
    info.version,
    info.checksum,
    info.fixed_marker_ok ? "yes" : "no",
    info.checksum_ok ? "yes" : "no"
  );
  std::fflush(stdout);
}

static auto ValidateROMData(std::vector<u8> const& file_data) -> ROMLoader::Result {
  auto size = file_data.size();

  if(size < sizeof(Header) || size > kMaxROMSize) {
    return ROMLoader::Result::BadImage;
  }

  auto header = ParseROMHeader(file_data);
  if(!header.fixed_marker_ok) {
    return ROMLoader::Result::BadImage;
  }

  return ROMLoader::Result::Success;
}

auto ROMLoader::Load(
  std::unique_ptr<CoreBase>& core,
  fs::path const& path,
  Config::BackupType backup_type,
  GPIODeviceType force_gpio
) -> Result {
  const auto save_path = fs::path{path}.replace_extension(".sav");

  return Load(core, path, save_path, backup_type, force_gpio);
}

auto ROMLoader::Load(
  std::unique_ptr<CoreBase>& core,
  fs::path const& rom_path,
  fs::path const& save_path,
  BackupType backup_type,
  GPIODeviceType force_gpio
) -> Result {
#if defined(PLATFORM_DREAMCAST)
  if(IsDreamcastVirtualPath(rom_path)) {
    DreamcastLoaderTrace("7A enter", rom_path);
    size_t size = 0;
    DreamcastLoaderTrace("7B stat begin", rom_path);
    auto size_status = GetFileSize(rom_path, size);
    if(size_status != Result::Success) {
      DreamcastLoaderTrace("7B stat failed", rom_path);
      return size_status;
    }
    DreamcastLoaderTrace("7B stat ok", rom_path, size);

    if(size < sizeof(Header) || size > kMaxROMSize) {
      DreamcastLoaderTrace("7B size invalid", rom_path, size);
      return Result::BadImage;
    }

    if(kDreamcastForceFlatSmallROMs && size <= kDreamcastFlatROMLimit) {
      DreamcastLoaderTrace("7C flat small-ROM selected", rom_path, size);

      auto file_data = std::vector<u8>{};
      DreamcastLoaderTrace("7D flat read begin", rom_path, size);
      auto read_status = ReadFile(rom_path, file_data);
      if(read_status != Result::Success) {
        DreamcastLoaderTrace("7D flat read failed", rom_path);
        return read_status;
      }
      DreamcastLoaderTrace("7D flat read ok", rom_path, file_data.size());

      DreamcastLoaderTrace("7E flat validate begin", rom_path, file_data.size());
      auto validation = ValidateROMData(file_data);
      if(validation != Result::Success) {
        DreamcastLoaderTrace("7E flat validate failed", rom_path, file_data.size());
        return validation;
      }
      DreamcastLoaderTrace("7E flat validate ok", rom_path, file_data.size());

      DreamcastLoaderTrace("7F header parse begin", rom_path, file_data.size());
      auto header_info = ParseROMHeader(file_data);
      TraceROMHeader(header_info, rom_path);
      auto game_info = header_info.game_info;
      DreamcastLoaderTrace("7F cartridge info ok", rom_path, file_data.size());

      if(backup_type == BackupType::Detect) {
        DreamcastLoaderTrace("7G backup detect begin", rom_path, file_data.size());
        if(game_info.backup_type != BackupType::Detect) {
          backup_type = game_info.backup_type;
        } else {
          backup_type = GetBackupType(file_data);
          if(backup_type == BackupType::Detect) {
            ATOM_WARN("ROMLoader: failed to detect backup type!");
            backup_type = BackupType::SRAM;
          }
        }
        DreamcastLoaderTrace("7G backup detect ok", rom_path, file_data.size());
      }

      DreamcastLoaderTrace("7H backup create begin", rom_path, file_data.size());
      auto backup = CreateBackup(core, save_path, backup_type);
      DreamcastLoaderTrace("7H backup create ok", rom_path, file_data.size());
      auto gpio = std::unique_ptr<GPIO>{};
      auto gpio_devices = game_info.gpio | force_gpio;

      if(gpio_devices != GPIODeviceType::None) {
        DreamcastLoaderTrace("7I gpio create begin", rom_path, file_data.size());
        gpio = std::make_unique<GPIO>();

        if(gpio_devices & GPIODeviceType::RTC) {
          gpio->Attach(core->CreateRTC());
        }

        if(gpio_devices & GPIODeviceType::SolarSensor) {
          gpio->Attach(core->CreateSolarSensor());
        }
        DreamcastLoaderTrace("7I gpio create ok", rom_path, file_data.size());
      }

      u32 rom_mask = u32(kMaxROMSize - 1);
      if(game_info.mirror) {
        rom_mask = u32(RoundSizeToPowerOfTwo(size) - 1);
      }

      DreamcastLoaderTrace("7J flat attach begin", rom_path, file_data.size());
      core->Attach(ROM{
        std::move(file_data),
        std::move(backup),
        std::move(gpio),
        rom_mask
      });
      DreamcastLoaderTrace("7J flat attach ok", rom_path, size);
      return Result::Success;
    }

    DreamcastLoaderTrace("7C paged open begin", rom_path, size);
    auto* file = std::fopen(rom_path.string().c_str(), "rb");
    if(!file) {
      DreamcastLoaderTrace("7C paged open failed", rom_path, size);
      return Result::CannotOpenFile;
    }
    DreamcastLoaderTrace("7C paged open ok", rom_path, size);

    auto header_data = std::vector<u8>(sizeof(Header));
    DreamcastLoaderTrace("7D header read begin", rom_path, size);
    const auto header_read = std::fread(header_data.data(), 1, header_data.size(), file);
    std::fclose(file);

    if(header_read != header_data.size()) {
      DreamcastLoaderTrace("7D header read failed", rom_path, header_read);
      return Result::CannotOpenFile;
    }
    DreamcastLoaderTrace("7D header read ok", rom_path, header_read);

    DreamcastLoaderTrace("7E header validate begin", rom_path, header_read);
    auto validation = ValidateROMData(header_data);
    if(validation != Result::Success) {
      DreamcastLoaderTrace("7E header validate failed", rom_path, header_read);
      return validation;
    }
    DreamcastLoaderTrace("7E header validate ok", rom_path, header_read);

    DreamcastLoaderTrace("7F header parse begin", rom_path, header_read);
    auto header_info = ParseROMHeader(header_data);
    TraceROMHeader(header_info, rom_path);
    auto game_info = header_info.game_info;
    DreamcastLoaderTrace("7F cartridge info ok", rom_path, header_read);

    if(backup_type == BackupType::Detect) {
      DreamcastLoaderTrace("7G backup detect begin", rom_path, size);
      if(game_info.backup_type != BackupType::Detect) {
        backup_type = game_info.backup_type;
      } else {
        backup_type = GetBackupTypeFromFile(rom_path, size);
        if(backup_type == BackupType::Detect) {
          ATOM_WARN("ROMLoader: failed to detect backup type!");
          backup_type = BackupType::SRAM;
        }
      }
      DreamcastLoaderTrace("7G backup detect ok", rom_path, size);
    }

    DreamcastLoaderTrace("7H backup create begin", rom_path, size);
    auto backup = CreateBackup(core, save_path, backup_type);
    DreamcastLoaderTrace("7H backup create ok", rom_path, size);
    auto gpio = std::unique_ptr<GPIO>{};
    auto gpio_devices = game_info.gpio | force_gpio;

    if(gpio_devices != GPIODeviceType::None) {
      DreamcastLoaderTrace("7I gpio create begin", rom_path, size);
      gpio = std::make_unique<GPIO>();

      if(gpio_devices & GPIODeviceType::RTC) {
        gpio->Attach(core->CreateRTC());
      }

      if(gpio_devices & GPIODeviceType::SolarSensor) {
        gpio->Attach(core->CreateSolarSensor());
      }
      DreamcastLoaderTrace("7I gpio create ok", rom_path, size);
    }

    u32 rom_mask = u32(kMaxROMSize - 1);
    if(game_info.mirror) {
      rom_mask = u32(RoundSizeToPowerOfTwo(size) - 1);
    }

    DreamcastLoaderTrace("7J paged attach begin", rom_path, size);
    core->Attach(ROM{
      rom_path.string(),
      size,
      std::move(backup),
      std::move(gpio),
      rom_mask
    });
    DreamcastLoaderTrace("7J paged attach ok", rom_path, size);

    // The ROM constructor opens the file for paged reads.  If fopen failed
    // (e.g. disc ejected between validation and load), the ROM object is
    // invalid; report the error rather than running with open-bus behavior.
    if(!core->GetROM().IsPagedROM()) {
      DreamcastLoaderTrace("7K paged reopen failed", rom_path, size);
      return Result::CannotOpenFile;
    }
    DreamcastLoaderTrace("7K paged reopen ok", rom_path, size);

    // Warm page 0 so the first CPU instruction fetch after Reset() hits the
    // cache instead of triggering disc I/O on the very first opcode.
    DreamcastLoaderTrace("7L preload begin", rom_path, size);
    if(!core->GetROM().PreloadFirstPage() || core->GetROM().HasReadError()) {
      DreamcastLoaderTrace("7L preload failed", rom_path, size);
      return Result::CannotOpenFile;
    }
    DreamcastLoaderTrace("7L preload ok", rom_path, size);

    return Result::Success;
  }
#endif

  auto file_data = std::vector<u8>{};
  auto read_status = ReadFile(rom_path, file_data);

  if(read_status != Result::Success) {
    return read_status;
  }

  auto size = file_data.size();

  auto validation = ValidateROMData(file_data);
  if(validation != Result::Success) {
    return validation;
  }

  auto header_info = ParseROMHeader(file_data);
  auto game_info = header_info.game_info;

  if(backup_type == BackupType::Detect) {
    if(game_info.backup_type != BackupType::Detect) {
      backup_type = game_info.backup_type;
    } else {
      backup_type = GetBackupType(file_data);
      if(backup_type == BackupType::Detect) {
        ATOM_WARN("ROMLoader: failed to detect backup type!");
        backup_type = BackupType::SRAM;
      }
    }
  }

  auto backup = CreateBackup(core, save_path, backup_type);

  auto gpio = std::unique_ptr<GPIO>{};

  auto gpio_devices = game_info.gpio | force_gpio;

  if(gpio_devices != GPIODeviceType::None) {
    gpio = std::make_unique<GPIO>();

    if(gpio_devices & GPIODeviceType::RTC) {
      gpio->Attach(core->CreateRTC());
    }

    if(gpio_devices & GPIODeviceType::SolarSensor) {
      gpio->Attach(core->CreateSolarSensor());
    }
  }

  u32 rom_mask = u32(kMaxROMSize - 1);
  if(game_info.mirror) {
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

auto ROMLoader::ReadFile(fs::path const& path, std::vector<u8>& file_data) -> Result {
#if defined(PLATFORM_DREAMCAST)
  const auto path_string = path.string();
  if(path_string.rfind("/cd/", 0) == 0 || path_string.rfind("/pc/", 0) == 0) {
    auto* file = std::fopen(path_string.c_str(), "rb");
    if(!file) {
      return Result::CannotOpenFile;
    }

    if(std::fseek(file, 0, SEEK_END) != 0) {
      std::fclose(file);
      return Result::CannotOpenFile;
    }

    const auto size = std::ftell(file);
    if(size < 0 || std::fseek(file, 0, SEEK_SET) != 0) {
      std::fclose(file);
      return Result::CannotOpenFile;
    }

    file_data.resize(static_cast<size_t>(size));
    const auto read = std::fread(file_data.data(), 1, file_data.size(), file);
    std::fclose(file);

    return read == file_data.size() ? Result::Success : Result::CannotOpenFile;
  }
#endif

  if(!fs::exists(path)) {
    return Result::CannotFindFile;
  }

  if(fs::is_directory(path)) {
    return Result::CannotOpenFile;
  }

  auto archive_result = ReadFileFromArchive(path, file_data);

  /* Forward result form ReadFileFromArchive() if the archive could be loaded,
   * and the GBA file was found and loaded or there was no GBA file.
   */
  if(archive_result == Result::BadImage ||
      archive_result == Result::Success) {
    return archive_result;
  }

  auto file_stream = std::ifstream{path, std::ios::binary};

  if(!file_stream.good()) {
    return Result::CannotOpenFile;
  }

  auto file_size = fs::file_size(path);

  file_data.resize(file_size);
  file_stream.read((char*)file_data.data(), file_size);
  if(file_stream.gcount() != static_cast<std::streamsize>(file_size)) {
    return Result::CannotOpenFile;
  }
  return Result::Success;
}

auto ROMLoader::ReadFileFromArchive(fs::path const& path, std::vector<u8>& file_data) -> Result {
  auto stream = ar_open_file(reinterpret_cast<const char*>(path.u8string().c_str()));

  if(!stream) {
    return Result::CannotOpenFile;
  }

  ar_archive* archive = ar_open_zip_archive(stream, false);

  if(!archive) archive = ar_open_rar_archive(stream);
  if(!archive) archive = ar_open_7z_archive(stream);
  if(!archive) archive = ar_open_tar_archive(stream);

  if(!archive) {
    ar_close(stream);
    return Result::CannotOpenFile;
  }

  auto result = Result::BadImage;

  while(ar_parse_entry(archive)) {
    auto filename = ar_entry_get_name(archive);
    auto extension = fs::path{filename}.extension();

    if(extension == ".gba" || extension == ".GBA") {
      auto size = ar_entry_get_size(archive);
      file_data.resize(size);
      ar_entry_uncompress(archive, file_data.data(), size);
      result = Result::Success;
      break;
    }
  }

  ar_close_archive(archive);
  ar_close(stream);
  return result;
}

auto ROMLoader::GetGameInfo(
  std::vector<u8>& file_data
) -> GameInfo {
  auto header = reinterpret_cast<Header*>(file_data.data());
  auto game_code = std::string{};
  game_code.assign(header->game.code, 4);

  auto db_entry = g_game_db.find(game_code);
  if(db_entry != g_game_db.end()) {
    return db_entry->second;
  }

  return GameInfo{};
}

auto ROMLoader::GetBackupType(
  std::vector<u8>& file_data
) -> BackupType {
  static constexpr std::pair<std::string_view, BackupType> signatures[6] {
    { "EEPROM_V",   BackupType::EEPROM_DETECT },
    { "SRAM_V",     BackupType::SRAM },
    { "SRAM_F_V",   BackupType::SRAM },
    { "FLASH_V",    BackupType::FLASH_64 },
    { "FLASH512_V", BackupType::FLASH_64 },
    { "FLASH1M_V",  BackupType::FLASH_128 }
  };

  const auto size = file_data.size();

  for(int i = 0; i < size; i += sizeof(u32)) {
    for(auto const& [signature, type] : signatures) {
      if((i + signature.size()) <= size &&
          std::memcmp(&file_data[i], signature.data(), signature.size()) == 0) {
        return type;
      }
    }
  }

  return BackupType::Detect;
}

auto ROMLoader::CreateBackup(
  std::unique_ptr<CoreBase>& core,
  fs::path const& save_path,
  BackupType backup_type
) -> std::unique_ptr<Backup> {
  switch(backup_type) {
    case BackupType::SRAM: return std::make_unique<SRAM>(save_path);
    case BackupType::FLASH_64: return std::make_unique<FLASH>(save_path, FLASH::SIZE_64K);
    case BackupType::FLASH_128: return std::make_unique<FLASH>(save_path, FLASH::SIZE_128K);
    case BackupType::EEPROM_4: return std::make_unique<EEPROM>(save_path, EEPROM::SIZE_4K, core->GetScheduler());
    case BackupType::EEPROM_64: return std::make_unique<EEPROM>(save_path, EEPROM::SIZE_64K, core->GetScheduler());
    case BackupType::EEPROM_DETECT: return std::make_unique<EEPROM>(save_path, EEPROM::DETECT, core->GetScheduler());
    default: assert(false);
  }

  return {};
}

auto ROMLoader::Validate(fs::path const& path) -> Result {
#if defined(PLATFORM_DREAMCAST)
  if(IsDreamcastVirtualPath(path)) {
    // Read only the ROM header to avoid loading the entire cartridge into memory.
    size_t size = 0;
    auto size_status = GetFileSize(path, size);
    if(size_status != Result::Success) {
      return size_status;
    }

    if(size < sizeof(Header) || size > kMaxROMSize) {
      return Result::BadImage;
    }

    auto* file = std::fopen(path.string().c_str(), "rb");
    if(!file) {
      return Result::CannotOpenFile;
    }

    auto header_data = std::vector<u8>(sizeof(Header));
    const auto read = std::fread(header_data.data(), 1, header_data.size(), file);
    std::fclose(file);

    if(read != header_data.size()) {
      return Result::CannotOpenFile;
    }

    return ValidateROMData(header_data);
  }
#endif

  auto file_data = std::vector<u8>{};
  auto read_status = ReadFile(path, file_data);

  if(read_status != Result::Success) {
    return read_status;
  }

  return ValidateROMData(file_data);
}

auto ROMLoader::GetFileSize(fs::path const& path, size_t& file_size) -> Result {
  file_size = 0;

#if defined(PLATFORM_DREAMCAST)
  const auto path_string = path.string();
  if(path_string.rfind("/cd/", 0) == 0 || path_string.rfind("/pc/", 0) == 0) {
    auto* file = std::fopen(path_string.c_str(), "rb");
    if(!file) {
      return Result::CannotOpenFile;
    }

    if(std::fseek(file, 0, SEEK_END) != 0) {
      std::fclose(file);
      return Result::CannotOpenFile;
    }

    const auto size = std::ftell(file);
    std::fclose(file);

    if(size < 0) {
      return Result::CannotOpenFile;
    }

    file_size = static_cast<size_t>(size);
    return Result::Success;
  }
#endif

  if(!fs::exists(path)) {
    return Result::CannotFindFile;
  }

  if(fs::is_directory(path)) {
    return Result::CannotOpenFile;
  }

  file_size = static_cast<size_t>(fs::file_size(path));
  return Result::Success;
}

auto ROMLoader::Describe(Result result) -> const char* {
  switch(result) {
    case Result::CannotFindFile: return "ROM file not found";
    case Result::CannotOpenFile: return "ROM file could not be read";
    case Result::BadImage: return "ROM image is invalid";
    case Result::Success: return "Success";
  }

  return "Unknown ROM loader error";
}

auto ROMLoader::RoundSizeToPowerOfTwo(size_t size) -> size_t {
  size_t pot_size = 1;

  while(pot_size < size) {
    pot_size *= 2;
  }

  return pot_size;
}

} // namespace nba
