/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#ifndef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif

#include <core/backup/eeprom.hpp>
#include <core/backup/flash.hpp>
#include <core/backup/sram.hpp>
#include <cstring>
#include <exception>
#include <experimental/filesystem>
#include <fstream>
#include <map>

#include "emulator.hpp"

namespace nba {

namespace fs = std::experimental::filesystem;

using namespace nba::core;

using SaveType = Config::SaveType;

constexpr int g_cycles_per_frame = 280896;
constexpr int g_bios_size = 0x4000;

Emulator::Emulator(std::shared_ptr<Config> config)
  : cpu(config)
  , config(config)
{
  save_detect = (config->save_type == SaveType::Detect);
  Reset();
}

void Emulator::Reset() { cpu.Reset(); }

auto Emulator::DetectSaveType(std::uint8_t* rom, size_t size) -> SaveType {
  const std::map<std::string, Config::SaveType> signatures {
    { "EEPROM_V",   SaveType::EEPROM_4  },
    { "SRAM_V",     SaveType::SRAM      },
    { "FLASH_V",    SaveType::FLASH_64  },
    { "FLASH512_V", SaveType::FLASH_64  },
    { "FLASH1M_V",  SaveType::FLASH_128 }
  };
  
  for (int i = 0; i < size; i += 4) {
    for (auto const& pair : signatures) {
      auto const& string = pair.first;
      
      if (std::memcmp(&rom[i], string.c_str(), string.size()) == 0) {
        return pair.second;
      }
    }
  }
  
  return Config::SaveType::Detect;
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
  
  if (!fs::exists(path) || !fs::is_regular_file(path)) {
    return StatusCode::GameNotFound;
  }
  
  size = fs::file_size(path);
  
  if (size > 32*1024*1024) {
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
    
  auto rom = std::make_unique<std::uint8_t[]>(size);
  stream.read((char*)(rom.get()), size);
  stream.close();
  
  std::string save_path = path.substr(0, path.find_last_of(".")) + ".sav";
  
  /* TODO: include a database to bust troublemakers. */
  if (save_detect) {
    config->save_type = DetectSaveType(rom.get(), size);
  }
  
  switch (config->save_type) {
    case SaveType::SRAM:
    case SaveType::Detect: // opt for SRAM if no save type was detected.
      cpu.memory.rom.backup = std::make_unique<SRAM>(save_path);
      break;
    case SaveType::FLASH_64:
      cpu.memory.rom.backup = std::make_unique<FLASH>(save_path, FLASH::SIZE_64K);
      break;
    case SaveType::FLASH_128:
      cpu.memory.rom.backup = std::make_unique<FLASH>(save_path, FLASH::SIZE_128K);
      break;
    case SaveType::EEPROM_4:
      cpu.memory.rom.backup = std::make_unique<EEPROM>(save_path, EEPROM::SIZE_4K);
      break;
    case SaveType::EEPROM_64:
      cpu.memory.rom.backup = std::make_unique<EEPROM>(save_path, EEPROM::SIZE_64K);
      break;
  }
  
  /* Handle ROM-mirroring for the classic NES titles. */
  if (size > 0xAC && rom[0xAC] == 'F') {
    std::uint32_t mask = 1;
    while (mask < size) {
      mask *= 2;
    }
    cpu.memory.rom.mask = mask - 1;
  } else {
    cpu.memory.rom.mask = 0x1FFFFFF;
  }
  
  cpu.memory.rom.data = std::move(rom);
  cpu.memory.rom.size = size;
  
  return StatusCode::Ok;
}

void Emulator::Frame() {
  cpu.RunFor(g_cycles_per_frame);
}

} // namespace nba
