// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "device/dc_input.hh"

#if NBA_DC_HAS_KOS
#include <dc/video.h>
#endif

namespace nba {

void DCInput::ClearKeys(CoreBase& core) {
  for(int i = 0; i < static_cast<int>(Key::Count); i++) {
    core.SetKeyStatus(static_cast<Key>(i), false);
  }
}

#if NBA_DC_HAS_KOS
auto DCInput::ReadControllerState() -> cont_state_t* {
  maple_device_t* cont = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
  if(!cont) {
    return nullptr;
  }

  return (cont_state_t*)maple_dev_status(cont);
}

auto DCInput::ButtonPressed(uint32 current, uint32 previous, uint32 mask) -> bool {
  return (current & mask) && !(previous & mask);
}
#endif

auto DCInput::PollInput(CoreBase& core) -> bool {
#if NBA_DC_HAS_KOS
  cont_state_t* state = ReadControllerState();
  if(!state) {
    ClearKeys(core);
    exit_combo_frames_ = 0;
    return false;
  }

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

auto DCInput::IsExitHintActive() const -> bool {
  return exit_combo_frames_ >= kExitHintFrames &&
         exit_combo_frames_ < kExitDebounceFrames;
}

auto DCInput::PollMenu(DCMenuInput& menu) -> void {
#if NBA_DC_HAS_KOS
  menu = {};

  cont_state_t* state = ReadControllerState();
  if(!state) {
    previous_buttons_ = 0xFFFF;
    previous_joyx_ = 0;
    previous_joyy_ = 0;
    return;
  }

  const uint32 current = state->buttons;
  menu.up = ButtonPressed(current, previous_buttons_, CONT_DPAD_UP);
  menu.down = ButtonPressed(current, previous_buttons_, CONT_DPAD_DOWN);
  menu.left = ButtonPressed(current, previous_buttons_, CONT_DPAD_LEFT);
  menu.right = ButtonPressed(current, previous_buttons_, CONT_DPAD_RIGHT);
  menu.confirm = ButtonPressed(current, previous_buttons_, CONT_A);
  menu.cancel = ButtonPressed(current, previous_buttons_, CONT_B);
  menu.settings = ButtonPressed(current, previous_buttons_, CONT_Y);
  menu.start = ButtonPressed(current, previous_buttons_, CONT_START);

  const bool analog_up = state->joyy < -kAnalogDeadZone;
  const bool analog_down = state->joyy > kAnalogDeadZone;
  const bool analog_left = state->joyx < -kAnalogDeadZone;
  const bool analog_right = state->joyx > kAnalogDeadZone;
  const bool prev_analog_up = previous_joyy_ < -kAnalogDeadZone;
  const bool prev_analog_down = previous_joyy_ > kAnalogDeadZone;
  const bool prev_analog_left = previous_joyx_ < -kAnalogDeadZone;
  const bool prev_analog_right = previous_joyx_ > kAnalogDeadZone;

  menu.up |= analog_up && !prev_analog_up;
  menu.down |= analog_down && !prev_analog_down;
  menu.left |= analog_left && !prev_analog_left;
  menu.right |= analog_right && !prev_analog_right;

  previous_buttons_ = current;
  previous_joyx_ = state->joyx;
  previous_joyy_ = state->joyy;
#else
  (void)menu;
#endif
}

auto DCInput::WaitForButton(Button button) -> void {
#if NBA_DC_HAS_KOS
  uint32 mask = CONT_START;
  if(button == Button::Start) {
    mask = CONT_START;
  }

  while(true) {
    cont_state_t* state = ReadControllerState();
    if(state && (state->buttons & mask)) {
      break;
    }

    vid_waitvbl();
  }

  while(true) {
    cont_state_t* state = ReadControllerState();
    if(!state || !(state->buttons & mask)) {
      break;
    }

    vid_waitvbl();
  }
#else
  (void)button;
#endif
}

} // namespace nba
