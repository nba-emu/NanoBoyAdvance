// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fstream>
#include <platform/writer/save_state.hh>

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
