// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hw/keypad/keypad.hh"

namespace nba::core {

void KeyPad::LoadState(SaveState const& state) {
  u16 keycnt = state.keycnt;

  control.mask = keycnt & 0x3FF;
  control.interrupt = keycnt & 0x4000;
  control.mode = (KeyControl::Mode)(keycnt >> 15);
}

void KeyPad::CopyState(SaveState& state) {
  state.keycnt = control.mask |
                (control.interrupt ? 0x4000 : 0) |
                ((int)control.mode << 15);
}

} // namespace nba::core
