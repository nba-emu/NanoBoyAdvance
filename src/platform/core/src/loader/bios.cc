// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <platform/loader/bios.hh>
#include <filesystem>
#include <fstream>
#include <vector>

namespace nba {

static constexpr size_t kBIOSSize = 0x4000;

auto BIOSLoader::Load(
  std::unique_ptr<CoreBase>& core,
  fs::path const& path
) -> Result {
  if(!fs::exists(path)) {
    return Result::CannotFindFile;
  }

  if(fs::is_directory(path)) {
    return Result::CannotOpenFile;
  }

  auto size = fs::file_size(path);
  if(size != kBIOSSize) {
    return Result::BadImage;
  }

  auto file_stream = std::ifstream{path.c_str(), std::ios::binary};
  if(!file_stream.good()) {
    return Result::CannotOpenFile;
  }

  std::vector<u8> file_data;
  file_data.resize(size);
  file_stream.read((char*)file_data.data(), size);
  file_stream.close();

  core->Attach(file_data);
  return Result::Success;
}

} // namespace nba
