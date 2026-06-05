// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <nba/core.hh>

#if defined(PLATFORM_DREAMCAST) && __has_include(<kos.h>)
#define NBA_DC_HAS_KOS 1
#include <kos.h>
#include <dc/maple.h>
#include <dc/maple/controller.h>
#else
#define NBA_DC_HAS_KOS 0
#endif

namespace nba {

// Maps Dreamcast controller buttons to GBA Key enums.
// PollInput() returns true once the exit combo has been held long enough.
struct DCInput {
  // ~25% of the analog stick range (±127).
  static constexpr int kAnalogDeadZone = 32;
  static constexpr int kExitDebounceFrames = 60;

  auto PollInput(CoreBase& core) -> bool;

private:
  static void ClearKeys(CoreBase& core);

  int exit_combo_frames_ = 0;
};

} // namespace nba
