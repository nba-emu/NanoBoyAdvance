/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <device/audio_device.hpp>

#include "SDL.h"

/* TODO: this probably should get a rework, it's neither efficient nor really nice. */
class SDL2_InputDevice : public nba::InputDevice {
public:
  auto Poll(Key key) -> bool final {
    auto keystate = SDL_GetKeyboardState(NULL);

    using Key = InputDevice::Key;

    switch (key) {
    case Key::A:
      return keystate[SDL_SCANCODE_Z] || keystate[SDL_SCANCODE_Y];
    case Key::B:
      return keystate[SDL_SCANCODE_X];
    case Key::Select:
      return keystate[SDL_SCANCODE_BACKSPACE];
    case Key::Start:
      return keystate[SDL_SCANCODE_RETURN];
    case Key::Right:
      return keystate[SDL_SCANCODE_RIGHT];
    case Key::Left:
      return keystate[SDL_SCANCODE_LEFT];
    case Key::Up:
      return keystate[SDL_SCANCODE_UP];
    case Key::Down:
      return keystate[SDL_SCANCODE_DOWN];
    case Key::R:
      return keystate[SDL_SCANCODE_S];
    case Key::L:
      return keystate[SDL_SCANCODE_A];
    }

    return false;
  }
};