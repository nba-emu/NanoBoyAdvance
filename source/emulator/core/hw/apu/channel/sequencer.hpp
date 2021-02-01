/*
 * Copyright (C) 2020 fleroviux
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
    if (length == 0) {
      length = default_length;
    }
  }

  bool Tick() {
    if (enabled) {
      return --length > 0;
    }
    return true;
  }

  int length;
  bool enabled;

private:
  int default_length;
};

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
    if (enabled) {
      current_freq = initial_freq;
      shadow_freq = initial_freq;
      step = divider;
      active = shift != 0 || divider != 0;
    }
  }

  bool Tick() {
    if (active && --step == 0) {
      int new_freq;
      int offset = shadow_freq >> shift;

      /* TODO: then frequency calculation and overflow check are run AGAIN immediately
       * using this new value, but this second new frequency is not written back.
       */
      step = divider;

      if (direction == Direction::Increment) {
        new_freq = shadow_freq + offset;
      } else {
        new_freq = shadow_freq - offset;
      }

      if (new_freq >= 2048) {
        return false;
      } else if (shift != 0) {
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
  int step;
};

class BaseChannel {
public:
  static constexpr int s_cycles_per_step = 16777216 / 512;

  BaseChannel(
    bool enable_envelope,
    bool enable_sweep,
    int default_length = 64
  )   : length(default_length) {
    envelope.enabled = enable_envelope;
    sweep.enabled = enable_sweep;
    Reset();
  }

  virtual bool IsEnabled() { return enabled; }

  void Reset() {
    length.Reset();
    envelope.Reset();
    sweep.Reset();
    enabled = false;
    step = 0;
  }

  void Tick() {
    // http://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Frame_Sequencer
    if ((step & 1) == 0) enabled &= length.Tick();
    if ((step & 3) == 2) enabled &= sweep.Tick();
    if (step == 7) envelope.Tick();

    step++;
    step &= 7;
  }

protected:
  void Restart() {
    length.Restart();
    sweep.Restart();
    envelope.Restart();
    enabled = true;
    step = 0;
  }

  void Disable() {
    enabled = false;
  }

  LengthCounter length;
  Envelope envelope;
  Sweep sweep;

private:
  bool enabled;
  int step;
};

} // namespace nba::core
