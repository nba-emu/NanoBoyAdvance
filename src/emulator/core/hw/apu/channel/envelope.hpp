/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

namespace nba::core {

class Envelope {
public:
  void Reset() {
    direction = Direction::Decrement;
    initial_volume = 0;
    divider = 0;
    Restart();
  }

  void Restart() {
    step = divider;
    current_volume = initial_volume;
    active = enabled;
  }

  void Tick() {
    if (--step == 0) {
      step = divider;
    
      if (active && divider != 0) {
        if (direction == Direction::Increment) {
          if (current_volume != 15) {
            current_volume++;
          } else {
            active = false;
          }
        } else {
          if (current_volume != 0) {
            current_volume--;
          } else {
            active = false;
          }
        }
      }
    }
  }

  bool active = false;
  bool enabled = false;

  enum Direction {
    Increment = 1,
    Decrement = 0
  } direction;

  int initial_volume;
  int current_volume;
  int divider;

private:
  int step;
};

} // namespace nba::core
