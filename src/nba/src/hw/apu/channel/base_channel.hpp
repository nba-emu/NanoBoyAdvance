/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/save_state.hpp>

#include "hw/apu/channel/length_counter.hpp"
#include "hw/apu/channel/envelope.hpp"
#include "hw/apu/channel/sweep.hpp"

namespace nba::core {

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
  virtual auto GetSample() -> s8 = 0;

  void Reset() {
    length.Reset();
    envelope.Reset();
    sweep.Reset();
    enabled = false;
    step = 0;
  }

  void Tick() {
    // http://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware#Frame_Sequencer
    if((step & 1) == 0) enabled &= length.Tick();
    if((step & 3) == 2) enabled &= sweep.Tick();
    if(step == 7) envelope.Tick();

    step++;
    step &= 7;
  }

  void LoadState(SaveState::APU::IO::PSG const& state);
  void CopyState(SaveState::APU::IO::PSG& state);

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
