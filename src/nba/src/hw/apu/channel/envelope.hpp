/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

namespace nba::core {

/**
 * TODO:
 * - Figure out how the envelope timer behaves when the envelope speed (divider) changes mid-note.
 *   - Is the new value loaded into the counter right away or on the next reload?
 * - Is the timer ticked when the envelope is deactivated (divider == 0)?
 *    - If so, what value is loaded into the timer on channel restart?
 */

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
    if(step == 1) {
      step = divider;

      if(active && divider != 0) {
        if(direction == Direction::Increment) {
          if(current_volume != 15) {
            current_volume++;
          } else {
            active = false;
          }
        } else {
          if(current_volume != 0) {
            current_volume--;
          } else {
            active = false;
          }
        }
      }
    } else {
      step = (step - 1) & 7;
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
  friend class BaseChannel;

  int step;
};

} // namespace nba::core
