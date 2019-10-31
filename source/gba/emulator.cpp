/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#include <exception>
#include <experimental/filesystem>
#include <fstream>

#include "emulator.hpp"

#include "core/backup/eeprom.hpp"

using namespace GameBoyAdvance;

namespace fs = std::experimental::filesystem;

constexpr int g_cycles_per_frame = 280896;
constexpr int g_bios_size = 0x4000;

Emulator::Emulator(std::shared_ptr<Config> config)
  : cpu(config) {
  Reset();
}

void Emulator::Reset() {
  cpu.Reset();
  
  /* TODO: exception is inconsistent, LoadGame returns a boolean. */
  
  auto bios_path = cpu.config->bios_path;
  
  if (!fs::exists(bios_path) || !fs::is_regular_file(bios_path))
    throw std::runtime_error("Cannot open BIOS file.");
  
  auto size = fs::file_size(bios_path);
  
  if (size != g_bios_size)
    throw std::runtime_error("BIOS file has wrong size.");
  
  std::ifstream stream { bios_path, std::ios::binary };
  
  if (!stream.good())
    throw std::runtime_error("Cannot open BIOS file.");
  
  stream.read((char*)cpu.memory.bios, g_bios_size);
  stream.close();
}

auto Emulator::LoadGame(std::string const& path) -> bool {
  size_t size;
  
  if (!fs::exists(path) || !fs::is_regular_file(path))
    return false;
  
  size = fs::file_size(path);
  
  if (size > 32*1024*1024)
    return false;
  
  std::ifstream stream { path, std::ios::binary };
  
  if (!stream.good())
    return false;
  
  auto rom = std::make_unique<std::uint8_t[]>(size);
  stream.read((char*)(rom.get()), size);
  stream.close();
  
  std::string save_path = path.substr(0, path.find_last_of(".")) + ".sav";
  
  cpu.memory.rom.data = std::move(rom);
  cpu.memory.rom.size = size;
  cpu.memory.rom.backup = std::make_shared<EEPROM>(save_path, EEPROM::SIZE_4K);
  
  return true;
}

void Emulator::Frame() {
  cpu.RunFor(g_cycles_per_frame);
}