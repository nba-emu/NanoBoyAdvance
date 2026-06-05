// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <platform/loader/bios.hh>
#include <filesystem>
#include <fstream>
#include <vector>

namespace nba {

static constexpr size_t kBIOSSize = 0x4000;

static auto ReadBIOSFile(fs::path const& path, std::vector<u8>& file_data) -> BIOSLoader::Result {
  if(!fs::exists(path)) {
    return BIOSLoader::Result::CannotFindFile;
  }

  if(fs::is_directory(path)) {
    return BIOSLoader::Result::CannotOpenFile;
  }

  auto size = fs::file_size(path);
  if(size != kBIOSSize) {
    return BIOSLoader::Result::BadImage;
  }

  auto file_stream = std::ifstream{path.c_str(), std::ios::binary};
  if(!file_stream.good()) {
    return BIOSLoader::Result::CannotOpenFile;
  }

  file_data.resize(size);
  file_stream.read(reinterpret_cast<char*>(file_data.data()), size);
  if(file_stream.gcount() != static_cast<std::streamsize>(size)) {
    return BIOSLoader::Result::CannotOpenFile;
  }

  return BIOSLoader::Result::Success;
}

auto BIOSLoader::Load(
  std::unique_ptr<CoreBase>& core,
  fs::path const& path
) -> Result {
  std::vector<u8> file_data;
  auto result = ReadBIOSFile(path, file_data);
  if(result != Result::Success) {
    return result;
  }

  core->Attach(file_data);
  return Result::Success;
}

auto BIOSLoader::Validate(fs::path const& path) -> Result {
  std::vector<u8> file_data;
  return ReadBIOSFile(path, file_data);
}

auto BIOSLoader::Describe(Result result) -> const char* {
  switch(result) {
    case Result::CannotFindFile: return "BIOS file not found";
    case Result::CannotOpenFile: return "BIOS file could not be read";
    case Result::BadImage: return "BIOS image is invalid (expected 16 KiB)";
    case Result::Success: return "Success";
  }

  return "Unknown BIOS loader error";
}

} // namespace nba
