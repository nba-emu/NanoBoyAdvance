/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

namespace GameBoyAdvance {

class InputDevice {
public:
  virtual ~InputDevice() {}

  enum class Key {
    Up,
    Down,
    Left,
    Right,
    Start,
    Select,
    A,
    B,
    L,
    R
  };
  
  virtual auto Poll(Key key) -> bool = 0;
};

class NullInputDevice : public InputDevice {
public:
  auto Poll(Key key) -> bool final {
    return false;
  }
};

}