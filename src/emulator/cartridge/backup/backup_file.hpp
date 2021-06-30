/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <algorithm>
#include <filesystem>
#include <common/integer.hpp>
#include <cstring>
#include <stdexcept>
#include <string>
#include <fstream>
#include <memory>
#include <vector>

namespace nba {

struct BackupFile {
  static auto OpenOrCreate(std::string const& save_path,
                           std::vector<size_t> const& valid_sizes,
                           int& default_size) -> std::unique_ptr<BackupFile> {
    namespace fs = std::filesystem;

    bool create = true;
    auto flags = std::ios::binary | std::ios::in | std::ios::out;
    std::unique_ptr<BackupFile> file { new BackupFile() };

    // TODO: check file type and permissions?
    if (fs::is_regular_file(save_path)) {
      auto size = fs::file_size(save_path);

      auto begin = valid_sizes.begin();
      auto end = valid_sizes.end();

      if (std::find(begin, end, size) != end) {
        file->stream.open(save_path, flags);
        if (file->stream.fail()) {
          throw std::runtime_error("BackupFile: unable to open file: " + save_path);
        }
        default_size = size;
        file->memory.reset(new u8[size]);
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
      file->memory.reset(new u8[default_size]);
      file->MemorySet(0, default_size, 0xFF);
    }

    return file;
  }

  auto Read(unsigned index) -> u8 {
    if (index >= file_size) {
      throw std::runtime_error("BackupFile: out-of-bounds index while reading.");
    }
    return memory[index];
  }

  void Write(unsigned index, u8 value) {
    if (index >= file_size) {
      throw std::runtime_error("BackupFile: out-of-bounds index while writing.");
    }
    memory[index] = value;
    if (auto_update) {
      Update(index, 1);
    }
  }

  void MemorySet(unsigned index, size_t length, u8 value) {
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
  std::unique_ptr<u8[]> memory;
};

} // namespace nba
