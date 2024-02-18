/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <filesystem>
#include <fstream>
#include <platform/loader/bios.hpp>
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
