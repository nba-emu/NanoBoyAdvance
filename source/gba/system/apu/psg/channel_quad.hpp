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

namespace GameBoyAdvance {

class QuadChannel {

public:
  QuadChannel(Scheduler& scheduler);
    
  void Reset();
  
  void Generate();
  auto Read (int offset) -> std::uint8_t;
  void Write(int offset, std::uint8_t value);
  
  std::int8_t sample = 0;
  
private:
  constexpr int GetSynthesisIntervalFromFrequency(int frequency) {
    // 128 cycles equals 131072 Hz, the highest possible frequency.
    // We are dividing by eight, because the waveform can change at
    // eight evenly spaced points inside a single cycle, depending on the wave duty.
    return 128 * (2048 - frequency) / 8;
  }

  Event event { 0, [this]() { this->Generate(); } };
  
  Sequencer sequencer;
  
  int phase;
  
  int  wave_duty;
  int  length;
  bool length_enable;
};

}