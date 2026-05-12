// SPDX-FileCopyrightText: Copyright 2026 The NanoBoyAdvance Authors
// SPDX-License-Identifier: GPL-3.0-or-later

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
    divider = 0;
    shift = 0;
    Restart();
  }

  void Restart() {
    /* TODO: If the sweep shift is non-zero, frequency calculation and the
     * overflow check are performed immediately.
     */
    if(enabled) {
      shadow_freq = current_freq;
      step = divider;
      active = shift != 0 || divider != 0;
    }
  }

  bool Tick() {
    if(active && --step == 0) {
      const int offset = shadow_freq >> shift;

      /* TODO: then frequency calculation and overflow check are run AGAIN immediately
       * using this new value, but this second new frequency is not written back.
       */
      step = divider;
      
      const int new_freq = shadow_freq + (direction == Direction::Increment ? offset : -offset);

      if(new_freq >= 2048) {
        return false;
      }

      if(shift != 0) {
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

  int current_freq;
  int shadow_freq;
  int divider;
  int shift;

private:
  friend class BaseChannel;

  int step;
};

} // namespace nba::core
