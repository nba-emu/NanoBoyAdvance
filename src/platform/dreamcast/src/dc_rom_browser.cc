// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dc_rom_browser.hh"

#include <platform/loader/rom.hh>

#include <algorithm>
#include <set>

namespace nba {

static auto AddDirectoryEntries(
  fs::path const& directory,
  std::set<fs::path>& seen,
  std::vector<ROMEntry>& entries
) -> void {
  if(directory.empty() || !fs::exists(directory) || !fs::is_directory(directory)) {
    return;
  }

  std::error_code error;
  for(auto const& entry : fs::directory_iterator(directory, error)) {
    if(!entry.is_regular_file()) {
      continue;
    }

    const auto path = entry.path();
    if(path.extension() != ".gba" && path.extension() != ".GBA") {
      continue;
    }

    if(!seen.insert(path).second) {
      continue;
    }

    if(ROMLoader::Validate(path) != ROMLoader::Result::Success) {
      continue;
    }

    entries.push_back(ROMEntry{path, path.filename().string()});
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
