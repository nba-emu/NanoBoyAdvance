/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <filesystem>
#include <fstream>
#include <platform/loader/save_state.hpp>

namespace fs = std::filesystem;

namespace nba {

auto SaveStateLoader::Load(
  std::unique_ptr<CoreBase>& core,
  std::string path
) -> Result {
  if (!fs::exists(path)) {
    return Result::CannotFindFile;
  }

  if (!fs::is_regular_file(path)) {
    return Result::CannotOpenFile;
  }

  auto file_size = fs::file_size(path);

  if (file_size != sizeof(SaveState)) {
    return Result::BadImage;
  }

  SaveState save_state;

  std::ifstream file_stream{path, std::ios::binary};

  if (!file_stream.good()) {
    return Result::CannotOpenFile;
  }

  file_stream.read((char*)&save_state, sizeof(SaveState));

  auto validate_result = Validate(save_state);

  if (validate_result != Result::Success) {
    return validate_result;
  }

  core->LoadState(save_state);
  return Result::Success;
}

auto SaveStateLoader::Validate(SaveState const& save_state) -> Result {
  if (save_state.magic != SaveState::kMagicNumber) {
    return Result::BadImage;
  }

  if (save_state.version != SaveState::kCurrentVersion) {
    return Result::UnsupportedVersion;
  }

  return Result::Success;
}

} // namespace nba
