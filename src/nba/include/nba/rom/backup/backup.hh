// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/integer.hh>
#include <nba/save_state.hh>

namespace nba {

struct Backup {
  virtual ~Backup() = default;

  virtual void Reset() = 0;
  virtual auto Read (u32 address) -> u8 = 0;
  virtual void Write(u32 address, u8 value) = 0;

  virtual void LoadState(SaveState const& state) = 0;
  virtual void CopyState(SaveState& state) = 0;

  // Whether save writes are persisted to disk as they happen.  Backends with
  // no on-disk file report true (nothing to lose); file-backed backends report
  // the state of their backing stream.
  virtual auto IsPersistent() const -> bool { return true; }

  // Best-effort persist of the current save contents to disk.  Returns true if
  // the save is safely on disk afterwards.
  virtual auto Flush() -> bool { return true; }
};

} // namespace nba
