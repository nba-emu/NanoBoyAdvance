/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#pragma once

#include "sequencer.hpp"

#include <cstdint>
#include <cstdio>

namespace GameBoyAdvance {

struct QuadChannel {
  QuadChannel(Scheduler& scheduler) {
    scheduler.Add(event);
    scheduler.Add(sequencer.event);
    Reset();
  }
  
  // TODO: find a better function name.
  constexpr int FreqCycles(int frequency) {
    // 128 cycles equals 131072 Hz, the highest possible frequency.
    // We are dividing by eight, because the waveform can change at
    // eight evenly spaced points, depending on the wave duty.
    return 128 * (2048 - frequency) / 8;
  }
  
  void Reset() {
    sequencer.Reset();
    
    phase = 0;
    event.countdown = FreqCycles(1800);
  }
  
  void Tick() {
    // Wave duty = 50%
    if (phase < 4) {
      sample = +127;
    } else {
      sample = -128;
    }
    
    phase = (phase + 1) % 8;
    
    event.countdown += FreqCycles(1800);
  }
  
  std::int8_t sample = 0;
  int phase;
  Sequencer sequencer;
  
  Event event { 0, [this]() { this->Tick(); } };
};

}