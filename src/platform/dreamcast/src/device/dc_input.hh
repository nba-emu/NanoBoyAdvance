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

struct DCMenuInput {
  bool up = false;
  bool down = false;
  bool left = false;
  bool right = false;
  bool confirm = false;
  bool cancel = false;
  bool settings = false;
  bool start = false;
};

// Maps Dreamcast controller buttons to GBA Key enums.
struct DCInput {
  // ~25% of the analog stick range (±127).
  static constexpr int kAnalogDeadZone = 32;
  static constexpr int kExitDebounceFrames = 60;
  static constexpr int kExitHintFrames = 30;

  enum class Button {
    Start
  };

  auto PollInput(CoreBase& core) -> bool;
  auto PollMenu(DCMenuInput& menu) -> void;
  auto WaitForButton(Button button) -> void;
  auto IsExitHintActive() const -> bool;

private:
  static void ClearKeys(CoreBase& core);

#if NBA_DC_HAS_KOS
  auto ReadControllerState() -> cont_state_t*;
  auto ButtonPressed(uint32 current, uint32 previous, uint32 mask) -> bool;

  uint32 previous_buttons_ = 0xFFFF;
  int8 previous_joyx_ = 0;
  int8 previous_joyy_ = 0;
#endif

  int exit_combo_frames_ = 0;
};

} // namespace nba
