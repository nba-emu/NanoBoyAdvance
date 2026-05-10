// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/rom/backup/backup.hh>
#include <nba/rom/backup/backup_file.hh>

namespace nba {

struct SRAM : Backup {
  SRAM(fs::path const& save_path);

  void Reset() final;
  auto Read (u32 address) -> u8 final;
  void Write(u32 address, u8 value) final;

  void LoadState(SaveState const& state) final;
  void CopyState(SaveState& state) final;

private:
  fs::path save_path;
  std::unique_ptr<BackupFile> file;
};

} // namespace nba
