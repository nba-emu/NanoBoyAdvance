// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dc_rom_browser.hh"

#include <platform/loader/rom.hh>

#include <algorithm>
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

  entries.push_back(ROMEntry{path, path.filename().string()});
}

static auto AddDirectoryEntries(
  fs::path const& directory,
  std::set<fs::path>& seen,
  std::vector<ROMEntry>& entries
) -> void {
#if defined(PLATFORM_DREAMCAST)
  const auto directory_string = directory.string();
  if(directory_string == "/cd") {
    static constexpr const char* kCDCandidates[] {
      "Kirby.gba"
    };

    for(auto const* candidate : kCDCandidates) {
      AddROMEntry(directory / candidate, seen, entries, false);
    }
    return;
  }

  if(auto* dir = opendir(directory_string.c_str())) {
    while(auto* entry = readdir(dir)) {
      if(std::strcmp(entry->d_name, ".") == 0 || std::strcmp(entry->d_name, "..") == 0) {
        continue;
      }

      const auto name = std::string{entry->d_name};
      if(!HasGBAExtension(name)) {
        continue;
      }

      AddROMEntry(directory / name, seen, entries);
    }

    closedir(dir);
    return;
  }
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
    if(fs::exists(last_path) &&
        ROMLoader::Validate(last_path) == ROMLoader::Result::Success &&
        seen.insert(last_path).second) {
      entries.push_back(ROMEntry{last_path, last_path.filename().string() + " (last)"});
    }
  }

  std::sort(entries.begin(), entries.end(), [](ROMEntry const& a, ROMEntry const& b) {
    return a.label < b.label;
  });

  return entries;
}

} // namespace nba
