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

#pragma once

#ifndef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif

#include <algorithm>
#include <experimental/filesystem>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <fstream>
#include <memory>
#include <vector>

namespace GameBoyAdvance {

class BackupFile {

public:
  
  static auto OpenOrCreate(std::string const& save_path,
                           std::vector<size_t> const& valid_sizes,
                           int& default_size) -> std::unique_ptr<BackupFile> {
    namespace fs = std::experimental::filesystem;
    
    bool create = true;
    auto flags = std::ios::binary | std::ios::in | std::ios::out;
    std::unique_ptr<BackupFile> file { new BackupFile() };
    
    /* TODO: check file type and permissions? */
    if (fs::exists(save_path)) {
      auto size = fs::file_size(save_path);

      auto begin = valid_sizes.begin();
      auto end = valid_sizes.end();
      
      if (std::find(begin, end, size) != end) {
        file->stream.open(save_path, flags);
        if (file->stream.fail()) {
          throw std::runtime_error("BackupFile: unable to open file: " + save_path);
        }
        default_size = size;
        file->memory.reset(new std::uint8_t[size]);
        file->stream.read((char*)file->memory.get(), size);
        create = false;
      }
    }

    file->file_size = default_size;
    
    /* A new save file is created either when no file exists yet,
     * or when the existing file has an invalid size.
     */
    if (create) {
      file->stream.open(save_path, flags | std::ios::trunc);
      if (file->stream.fail()) {
        throw std::runtime_error("BackupFile: unable to create file: " + save_path);
      }
      file->memory.reset(new std::uint8_t[default_size]);
      file->MemorySet(0, default_size, 0xFF);
    }
    
    return file;
  }
  
  auto Read(unsigned index) -> std::uint8_t {
    if (index >= file_size) {
      throw std::runtime_error("BackupFile: out-of-bounds index while reading.");
    }
    return memory[index];
  }
  
  void Write(unsigned index, std::uint8_t value) {
    if (index >= file_size) {
      throw std::runtime_error("BackupFile: out-of-bounds index while writing.");
    }
    memory[index] = value;
    if (auto_update) {
      Update(index, 1);
    }
  }
  
  void MemorySet(unsigned index, size_t length, std::uint8_t value) {
    if ((index + length) > file_size) {
      throw std::runtime_error("BackupFile: out-of-bounds index while setting memory.");
    }
    std::memset(&memory[index], value, length);
    if (auto_update) {
      Update(index, length);
    }
  }
  
  void Update(unsigned index, size_t length) {
    if ((index + length) > file_size) {
      throw std::runtime_error("BackupFile: out-of-bounds index while updating file.");
    }
    stream.seekg(index);
    stream.write((char*)&memory[index], length);
  }
  
  bool auto_update = true;
  
private:
  BackupFile() { }
  
  size_t file_size;
  std::fstream stream;
  std::unique_ptr<std::uint8_t[]> memory;
}; 

}
