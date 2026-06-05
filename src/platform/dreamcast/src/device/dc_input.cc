// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "device/dc_input.hh"

namespace nba {

void DCInput::ClearKeys(CoreBase& core) {
  for(int i = 0; i < static_cast<int>(Key::Count); i++) {
    core.SetKeyStatus(static_cast<Key>(i), false);
  }
}

auto DCInput::PollInput(CoreBase& core) -> bool {
#if NBA_DC_HAS_KOS
  maple_device_t* cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
  if(!cont) {
    ClearKeys(core);
    exit_combo_frames_ = 0;
    return false;
  }

  cont_state_t* state = (cont_state_t*)maple_dev_status(cont);
  if(!state) {
    ClearKeys(core);
    exit_combo_frames_ = 0;
    return false;
  }

  // Map Dreamcast buttons to GBA keys.
  // DC A → GBA A, DC B → GBA B, DC X → GBA L, DC Y → GBA R
  // DC Start → GBA Start, DC D-pad → GBA D-pad
  // Analog stick also mapped to D-pad with a dead zone.
  core.SetKeyStatus(Key::A,      state->buttons & CONT_A);
  core.SetKeyStatus(Key::B,      state->buttons & CONT_B);
  core.SetKeyStatus(Key::L,      state->buttons & CONT_X);
  core.SetKeyStatus(Key::R,      state->buttons & CONT_Y);
  core.SetKeyStatus(Key::Start,  state->buttons & CONT_START);
  core.SetKeyStatus(Key::Select, state->buttons & CONT_D);

  bool left  = (state->buttons & CONT_DPAD_LEFT)  || (state->joyx < -kAnalogDeadZone);
  bool right = (state->buttons & CONT_DPAD_RIGHT) || (state->joyx >  kAnalogDeadZone);
  bool up    = (state->buttons & CONT_DPAD_UP)    || (state->joyy < -kAnalogDeadZone);
  bool down  = (state->buttons & CONT_DPAD_DOWN)  || (state->joyy >  kAnalogDeadZone);

  core.SetKeyStatus(Key::Left,  left && !right);
  core.SetKeyStatus(Key::Right, right && !left);
  core.SetKeyStatus(Key::Up,    up && !down);
  core.SetKeyStatus(Key::Down,  down && !up);

  const uint32 exit_combo = CONT_START | CONT_A | CONT_B | CONT_X | CONT_Y;
  if((state->buttons & exit_combo) == exit_combo) {
    exit_combo_frames_++;
  } else {
    exit_combo_frames_ = 0;
  }

  return exit_combo_frames_ >= kExitDebounceFrames;
#else
  (void)core;
  return false;
#endif
}

} // namespace nba
