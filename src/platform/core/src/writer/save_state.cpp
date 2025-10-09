/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <fstream>
#include <platform/writer/save_state.hpp>

namespace nba {

auto SaveStateWriter::Write(
  std::unique_ptr<CoreBase>& core,
  fs::path const& path
) -> Result {
  std::ofstream file_stream{path.c_str(), std::ios::binary};

  if(!file_stream.good()) {
    return Result::CannotOpenFile;
  }

  SaveState save_state;
  core->CopyState(save_state);

  file_stream.write((const char*)&save_state, sizeof(SaveState));
  
  if(!file_stream.good()) {
    return Result::CannotWrite;
  }

  return Result::Success;
}

} // namespace nba