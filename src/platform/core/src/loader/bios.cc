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
  if(file_stream.gcount() != static_cast<std::streamsize>(size)) {
    return Result::CannotOpenFile;
  }
  file_stream.close();

  core->Attach(file_data);
  return Result::Success;
}

auto BIOSLoader::Validate(fs::path const& path) -> Result {
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
  if(file_stream.gcount() != static_cast<std::streamsize>(size)) {
    return Result::CannotOpenFile;
  }

  return Result::Success;
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
