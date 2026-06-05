// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/integer.hh>
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace nba {

struct BackupFile {
  static auto OpenOrCreate(fs::path const& save_path,
                           std::vector<size_t> const& valid_sizes,
                           int& default_size) -> std::unique_ptr<BackupFile> {
    bool create = true;
    auto flags = std::ios::binary | std::ios::in | std::ios::out;
    std::unique_ptr<BackupFile> file { new BackupFile() };

#if defined(PLATFORM_DREAMCAST)
    const auto save_path_string = save_path.string();
    if(save_path_string.rfind("/pc/", 0) == 0) {
      file->save_size = default_size;
      file->memory.reset(new u8[default_size]);
      file->auto_update = false;
      file->MemorySet(0, default_size, 0xFF);
      return file;
    }
#endif

    // @todo: check file type and permissions?
    if(fs::is_regular_file(save_path)) {
      auto file_size = fs::file_size(save_path);

      // allow for some extra/unused data; required for mGBA save compatibility
      auto save_size = file_size & ~63u;

      auto begin = valid_sizes.begin();
      auto end = valid_sizes.end();

      if(std::find(begin, end, save_size) != end) {
        file->stream.open(save_path.c_str(), flags);
        if(file->stream.fail()) {
          throw std::runtime_error("BackupFile: unable to open file: " + save_path.string());
        }
        default_size = save_size;
        file->save_size = save_size;
        file->memory.reset(new u8[file_size]);
        file->stream.read((char*)file->memory.get(), file_size);
        create = false;
      }
    }

    /* A new save file is created either when no file exists yet,
     * or when the existing file has an invalid size.
     */
    if(create) {
      file->save_size = default_size;
      file->stream.open(save_path, flags | std::ios::trunc);
      if(file->stream.fail()) {
        throw std::runtime_error("BackupFile: unable to create file: " + save_path.string());
      }
      file->memory.reset(new u8[default_size]);
      file->MemorySet(0, default_size, 0xFF);
    }

    return file;
  }

  auto Read(unsigned index) -> u8 {
    if(index >= save_size) {
      throw std::runtime_error("BackupFile: out-of-bounds index while reading.");
    }
    return memory[index];
  }

  void Write(unsigned index, u8 value) {
    if(index >= save_size) {
      throw std::runtime_error("BackupFile: out-of-bounds index while writing.");
    }
    memory[index] = value;
    if(auto_update) {
      Update(index, 1);
    }
  }

  void MemorySet(unsigned index, size_t length, u8 value) {
    if((index + length) > save_size) {
      throw std::runtime_error("BackupFile: out-of-bounds index while setting memory.");
    }
    std::memset(&memory[index], value, length);
    if(auto_update) {
      Update(index, length);
    }
  }

  void Update(unsigned index, size_t length) {
    if((index + length) > save_size) {
      throw std::runtime_error("BackupFile: out-of-bounds index while updating file.");
    }
    stream.seekg(index);
    stream.write((char*)&memory[index], length);
  }

  auto Buffer() -> u8* {
    return memory.get();
  }

  auto Size() -> size_t {
    return save_size;
  }

  bool auto_update = true;

private:
  BackupFile() { }

  size_t save_size;
  std::fstream stream;
  std::unique_ptr<u8[]> memory;
};

} // namespace nba
