// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <nba/rom/backup/sram.hh>

namespace nba {

SRAM::SRAM(fs::path const& save_path)
    : save_path(save_path) {
  Reset();
}

void SRAM::Reset() {
  int bytes = 32768;
  file = BackupFile::OpenOrCreate(save_path, { 32768 }, bytes);
}

auto SRAM::Read(u32 address) -> u8 {
  return file->Read(address & 0x7FFF);
}

void SRAM::Write(u32 address, u8 value) {
  file->Write(address & 0x7FFF, value);
}

} // namespace nba
