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

#include <cstdint>

#include "sequencer.hpp"
#include "../registers.hpp"

namespace GameBoyAdvance {

class NoiseChannel {
public:
  NoiseChannel(Scheduler& scheduler, BIAS& bias);
  
  void Reset();
  
  void Generate();
  auto Read (int offset) -> std::uint8_t;
  void Write(int offset, std::uint8_t value);
  
  std::int8_t sample = 0;
  
private:
  constexpr int GetSynthesisInterval(int ratio, int shift) {
    int interval = 64 << shift;
    
    if (ratio == 0) {
      interval /= 2;
    } else {
      interval *= ratio;
    }
    
    return interval;
  }
  
  std::uint16_t lfsr;
  
  Event event { 0, [this]() { this->Generate(); } };
  
  Sequencer sequencer;
  
  int  frequency_shift;
  int  frequency_ratio;
  int  width;
  int  length;
  bool length_enable;
  
  BIAS& bias;
  int skip_count;
};

}