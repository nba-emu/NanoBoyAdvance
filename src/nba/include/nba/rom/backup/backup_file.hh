// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/integer.hh>
#include <algorithm>
#include <cstdio>
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
      // On Dreamcast /pc/ paths std::filesystem is unreliable; use the C file
      // API which works through KOS's virtual filesystem layer.
      file->save_size = static_cast<size_t>(default_size);
      file->memory.reset(new u8[default_size]);
      std::memset(file->memory.get(), 0xFF, default_size);
      file->auto_update = false; // default; upgraded below if stream opens

      // Attempt to load an existing save file.
      bool loaded_existing = false;
      if(auto* f = std::fopen(save_path_string.c_str(), "rb")) {
        std::fseek(f, 0, SEEK_END);
        const long fsz = std::ftell(f);
        if(fsz > 0) {
          const size_t save_size = static_cast<size_t>(fsz) & ~size_t(63);
          const auto begin = valid_sizes.begin();
          const auto end   = valid_sizes.end();
          if(std::find(begin, end, save_size) != end) {
            std::fseek(f, 0, SEEK_SET);
            std::fread(file->memory.get(), 1, save_size, f);
            file->save_size = save_size;
            default_size = static_cast<int>(save_size);
            loaded_existing = true;
          }
        }
        std::fclose(f);
      }

      // Try to open the stream for write-back.  On real KallistiOS hardware
      // this works (std::fstream delegates to fopen); on Flycast the stream
      // may fail to open, in which case save data remains in-memory only.
      //
      // If we loaded an existing valid save, open in-place without truncating
      // so we only rewrite bytes that actually change (via Update / auto_update).
      // If no valid save existed, truncate to create a fresh file.
      if(loaded_existing) {
        file->stream.open(save_path_string.c_str(), flags);
      } else {
        file->stream.open(save_path_string.c_str(), flags | std::ios::trunc);
        if(file->stream.good()) {
          // Write the initial 0xFF-filled block so the file has the right size.
          file->stream.write(
            reinterpret_cast<const char*>(file->memory.get()),
            static_cast<std::streamsize>(file->save_size)
          );
        }
      }

      if(file->stream.good()) {
        file->auto_update = true;
      }

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
    stream.seekp(static_cast<std::streamoff>(index));
    stream.write(reinterpret_cast<const char*>(&memory[index]), static_cast<std::streamsize>(length));
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
