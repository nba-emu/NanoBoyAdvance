/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

namespace nba::core {

class LengthCounter {
public:
  LengthCounter(int default_length) : default_length(default_length) {
    Reset();
  }

  void Reset() {
    enabled = false;
    length = 0;
  }

  void Restart() {
    if(length == 0) {
      length = default_length;
    }
  }

  bool Tick() {
    if(enabled) {
      return --length > 0;
    }
    return true;
  }

  int length;
  bool enabled;

private:
  int default_length;
};

} // namespace nba::core
