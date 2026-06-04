// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "device/dc_input.hh"

namespace nba {

void DCInput::PollInput(CoreBase& core) {
#if NBA_DC_HAS_KOS
  maple_device_t* cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
  if(!cont) return;

  cont_state_t* state = (cont_state_t*)maple_dev_status(cont);
  if(!state) return;

  // Map Dreamcast buttons to GBA keys.
  // DC A → GBA A, DC B → GBA B, DC X → GBA L, DC Y → GBA R
  // DC Start → GBA Start, DC D-pad → GBA D-pad
  // Analog stick also mapped to D-pad with a dead zone.
  core.SetKeyStatus(Key::A,      state->buttons & CONT_A);
  core.SetKeyStatus(Key::B,      state->buttons & CONT_B);
  core.SetKeyStatus(Key::L,      state->buttons & CONT_X);
  core.SetKeyStatus(Key::R,      state->buttons & CONT_Y);
  core.SetKeyStatus(Key::Start,  state->buttons & CONT_START);
  core.SetKeyStatus(Key::Select, state->buttons & CONT_D); // DC D button as Select
  core.SetKeyStatus(Key::Up,     state->buttons & CONT_DPAD_UP);
  core.SetKeyStatus(Key::Down,   state->buttons & CONT_DPAD_DOWN);
  core.SetKeyStatus(Key::Left,   state->buttons & CONT_DPAD_LEFT);
  core.SetKeyStatus(Key::Right,  state->buttons & CONT_DPAD_RIGHT);

  // Analog stick → D-pad fallback (dead zone of 32)
  constexpr int kDeadZone = 32;

  if(state->joyx < -kDeadZone) {
    core.SetKeyStatus(Key::Left, true);
  } else if(state->joyx > kDeadZone) {
    core.SetKeyStatus(Key::Right, true);
  }

  if(state->joyy < -kDeadZone) {
    core.SetKeyStatus(Key::Up, true);
  } else if(state->joyy > kDeadZone) {
    core.SetKeyStatus(Key::Down, true);
  }
#else
  (void)core;
#endif
}

} // namespace nba
