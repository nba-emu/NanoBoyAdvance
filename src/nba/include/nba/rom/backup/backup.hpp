// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/integer.hpp>
#include <nba/save_state.hpp>

namespace nba {

struct Backup {
  virtual ~Backup() = default;

  virtual void Reset() = 0;
  virtual auto Read (u32 address) -> u8 = 0;
  virtual void Write(u32 address, u8 value) = 0;

  virtual void LoadState(SaveState const& state) = 0;
  virtual void CopyState(SaveState& state) = 0;
};

} // namespace nba
