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

/* TODO: the noise channel seems to be a real performance hog.
 * I guess I have to make the timing less tight to make it fast.
 */

namespace GameBoyAdvance {

class NoiseChannel {

public:
  NoiseChannel(Scheduler& scheduler) {
    sequencer.sweep.enabled = false;
    sequencer.envelope.enabled = true;

    scheduler.Add(event);
    scheduler.Add(sequencer.event);
    Reset();
  }
  
  void Reset() {
    sequencer.Reset();
    
    frequency = 0;
    sound_length = 0;
    divide_ratio = 0;
    size_flag = false;
    apply_length = false;
    lfsr = 0;
    sample = 0;
    
    event.countdown = GetSynthesisInterval(7, 15);
  }
  
  void Generate() {
    if (apply_length && sequencer.length >= (64 - sound_length)) {
      sample = 0;
      event.countdown = GetSynthesisInterval(7, 15);
      return;
    }
  
    constexpr std::uint16_t lfsr_xor[2] = { 0x6000, 0x60 };
    
    int carry = lfsr & 1;
    
    lfsr >>= 1;
    if (carry) {
      lfsr ^= lfsr_xor[size_flag];
      sample = +8;
    } else {
      sample = -8;
    }
    
    sample *= sequencer.envelope.current_volume;
    
    event.countdown += GetSynthesisInterval(divide_ratio, frequency);
  }
  
  auto Read(int offset) -> std::uint8_t {
    auto& envelope = sequencer.envelope;
    
    switch (offset) {
      /* Length / Envelope */
      case 0: return 0;
      case 1: {
        return envelope.divider |
              ((int)envelope.direction << 3) |
              (envelope.initial_volume << 4);
      }
      case 2: 
      case 3: return 0;

      /* Frequency / Control */
      case 4: {
        return divide_ratio |
              (size_flag ? 8 : 0) |
              (frequency << 4);
      }
      case 5: {
        return apply_length ? 0x40 : 0;
      }
                
      default: return 0;
    }
  }
  
  void Write(int offset, std::uint8_t value) {
    auto& envelope = sequencer.envelope;
    
    switch (offset) {
      /* Length / Envelope */
      case 0: {
        sound_length = value & 63;
        break;
      }
      case 1: {
        envelope.divider = value & 7;
        envelope.direction = Envelope::Direction((value >> 3) & 1);
        envelope.initial_volume = value >> 4;
        break;
      }

      /* Frequency / Control */
      case 4: {
        divide_ratio = value  & 7;
        size_flag = value  & 8;
        frequency = value >> 4;
        break;
      }
      case 5: {
        apply_length = value & 0x40;

        if (value & 0x80) {
          const std::uint16_t lfsr_init[] = { 0x4000, 0x0040 };
          
          sequencer.Restart();
          lfsr = lfsr_init[size_flag];
          
//          const u16 lfsr_init[] = { 0x4000, 0x0040 };
//          internal.output          = 0;
//          internal.lfsr            = lfsr_init[size_flag];
//          internal.volume          = envelope.initial;
//          internal.shift_cycles    = 0;
//          internal.length_cycles   = 0;
//          internal.envelope_cycles = 0;
        }
        break;
      }
    }
  }
  
  std::int8_t sample = 0;
  
private:
  constexpr int GetSynthesisInterval(int r, int s) {
    int interval = 32 * (2<<s); // 64 << s
    
    if (r == 0) {
      interval /= 2;
    } else {
      interval *= r;
    }
    
    return interval;
  }
  
  Event event { 0, [this]() { this->Generate(); } };
  
  Sequencer sequencer;
  
  /* TODO: rename these properly. */
  int frequency;
  int sound_length;
  int divide_ratio;
  bool size_flag;
  bool apply_length;
  
  std::uint16_t lfsr;
};

}