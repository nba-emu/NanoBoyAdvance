/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

namespace nba {

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

  static constexpr int kKeyCount = 10;
  
  virtual auto Poll(Key key) -> bool = 0;
};

class NullInputDevice : public InputDevice {
public:
  auto Poll(Key key) -> bool final {
    return false;
  }
};

class BasicInputDevice : public InputDevice {
public:
  void SetKeyStatus(Key key, bool pressed) {
    key_status[static_cast<int>(key)] = pressed;
  }

  auto Poll(Key key) -> bool final {
    return key_status[static_cast<int>(key)];
  }

private:
  bool key_status[kKeyCount];
};

} // namespace nba
