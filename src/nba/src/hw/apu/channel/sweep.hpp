/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

namespace nba::core {

/**
 * TODO: figure out specifics of how the sweep timer works when the sweep speed (divider) is set to zero
 * or changed in the middle of a note.
 */

class Sweep {
public:
  void Reset() {
    direction = Direction::Increment;
    initial_freq = 0;
    divider = 0;
    shift = 0;
    Restart();
  }

  void Restart() {
    /* TODO: If the sweep shift is non-zero, frequency calculation and the
     * overflow check are performed immediately.
     */
    if(enabled) {
      current_freq = initial_freq;
      shadow_freq = initial_freq;
      step = divider;
      active = shift != 0 || divider != 0;
    }
  }

  bool Tick() {
    if(active && --step == 0) {
      int new_freq;
      int offset = shadow_freq >> shift;

      /* TODO: then frequency calculation and overflow check are run AGAIN immediately
       * using this new value, but this second new frequency is not written back.
       */
      step = divider;

      if(direction == Direction::Increment) {
        new_freq = shadow_freq + offset;
      } else {
        new_freq = shadow_freq - offset;
      }

      if(new_freq >= 2048) {
        return false;
      } else if(shift != 0) {
        shadow_freq  = new_freq;
        current_freq = new_freq;
      }
    }

    return true;
  }

  bool active = false;
  bool enabled = false;

  enum Direction {
    Increment = 0,
    Decrement = 1
  } direction;

  int initial_freq;
  int current_freq;
  int shadow_freq;
  int divider;
  int shift;

private:
  friend class BaseChannel;

  int step;
};

} // namespace nba::core
