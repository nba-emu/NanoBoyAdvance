// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dc_rom_browser.hh"

#include <platform/loader/rom.hh>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <set>

namespace nba {

static auto HasGBAExtension(std::string const& path) -> bool {
  const auto dot = path.find_last_of('.');
  if(dot == std::string::npos) {
    return false;
  }

  auto extension = path.substr(dot);
  const auto version = extension.find(';');
  if(version != std::string::npos) {
    extension.resize(version);
  }

  return extension == ".gba" || extension == ".GBA";
}

static auto AddROMEntry(
  fs::path const& path,
  std::set<fs::path>& seen,
  std::vector<ROMEntry>& entries,
  bool validate = true
) -> void {
  if(!seen.insert(path).second) {
    return;
  }

  if(validate && ROMLoader::Validate(path) != ROMLoader::Result::Success) {
    return;
  }

  // Strip the ISO9660 version suffix (e.g. ";1") from the display label so
  // disc filenames like "GAME.GBA;1" appear as "GAME.GBA" in the menu.
  auto label = path.filename().string();
  const auto semicolon = label.rfind(';');
  if(semicolon != std::string::npos) {
    label.resize(semicolon);
  }

  entries.push_back(ROMEntry{path, std::move(label)});
}

static auto AddDirectoryEntries(
  fs::path const& directory,
  std::set<fs::path>& seen,
  std::vector<ROMEntry>& entries
) -> void {
#if defined(PLATFORM_DREAMCAST)
  const auto directory_string = directory.string();

  // Use POSIX opendir/readdir for all Dreamcast paths.  On real KallistiOS
  // hardware this enumerates both /pc and /cd correctly.  ISO9660 discs may
  // expose filenames with version suffixes (e.g. "GAME.GBA;1"); strip the
  // suffix from the path so that fopen always receives a clean name.
  if(auto* dir = opendir(directory_string.c_str())) {
    while(auto* entry = readdir(dir)) {
      if(std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0) {
        continue;
      }

      auto name = std::string{entry->d_name};

      // Strip ISO9660 version suffix from the filename used as the file path.
      const auto semicolon_pos = name.rfind(';');
      if(semicolon_pos != std::string::npos) {
        name.resize(semicolon_pos);
      }

      if(!HasGBAExtension(name)) {
        continue;
      }

      AddROMEntry(directory / name, seen, entries);
    }

    closedir(dir);
    return;
  }
  // opendir returned null; directory is inaccessible or does not exist.
  return;
#endif

  if(directory.empty() || !fs::exists(directory) || !fs::is_directory(directory)) {
    return;
  }

  std::error_code error;
  for(auto const& entry : fs::directory_iterator(directory, error)) {
    if(!entry.is_regular_file()) {
      continue;
    }

    const auto path = entry.path();
    if(!HasGBAExtension(path.filename().string())) {
      continue;
    }

    AddROMEntry(path, seen, entries);
  }
}

auto ROMBrowser::Scan(DreamcastConfig const& config) -> std::vector<ROMEntry> {
  std::set<fs::path> seen;
  std::vector<ROMEntry> entries;

  AddDirectoryEntries(config.rom_folder, seen, entries);
  AddDirectoryEntries("/cd", seen, entries);

  if(!config.last_rom.empty()) {
    const fs::path last_path{config.last_rom};

    // For Dreamcast virtual paths (/cd, /pc) std::filesystem::exists may
    // behave incorrectly because those paths are backed by KOS filesystem
    // drivers rather than the host VFS.  Use fopen as a portable existence
    // probe instead.
    bool last_rom_exists = false;
#if defined(PLATFORM_DREAMCAST)
    {
      const auto s = last_path.string();
      if(s.rfind("/cd/", 0) == 0 || s.rfind("/pc/", 0) == 0) {
        if(auto* f = std::fopen(s.c_str(), "rb")) {
          std::fclose(f);
          last_rom_exists = true;
        }
      } else {
        last_rom_exists = fs::exists(last_path);
      }
    }
#else
    last_rom_exists = fs::exists(last_path);
#endif

    if(last_rom_exists &&
        ROMLoader::Validate(last_path) == ROMLoader::Result::Success &&
        seen.insert(last_path).second) {
      auto last_label = last_path.filename().string();
      // Strip any ISO9660 version suffix from the label.
      const auto sc = last_label.rfind(';');
      if(sc != std::string::npos) {
        last_label.resize(sc);
      }
      entries.push_back(ROMEntry{last_path, std::move(last_label) + " (last)"});
    }
  }

  std::sort(entries.begin(), entries.end(), [](ROMEntry const& a, ROMEntry const& b) {
    return a.label < b.label;
  });

  return entries;
}

} // namespace nba
