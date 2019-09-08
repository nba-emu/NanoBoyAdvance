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

struct QuadChannel {
  QuadChannel(Scheduler& scheduler) {
    sequencer.sweep.enabled = true;
    sequencer.envelope.enabled = true;
    
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
    
    wave_duty = 0;
    length = 0;
    length_enable = false;
    
    phase = 0;
    event.countdown = FreqCycles(0);
  }
  
  void Tick() {
    if (length_enable && sequencer.length >= length) {
      sample = 0;
      // BIG TODO: event latency!
      event.countdown = FreqCycles(0);
      return;
    }
    
    int duty;
    
    switch (wave_duty) {
      case 0: duty = 1; break;
      case 1: duty = 2; break;
      case 2: duty = 4; break;
      case 3: duty = 6; break;
    }
    
    // TODO: pick correct wave duty.
    if (phase < duty) {
      sample = +127;
    } else {
      sample = -128;
    }
    
    std::int16_t hack = sample;
    hack *= sequencer.envelope.current_volume;
    hack /= 16; // 15?
    sample = std::int8_t(hack);
    
    
    phase = (phase + 1) % 8;
    
    event.countdown += FreqCycles(sequencer.sweep.current_freq);
  }
  
  auto Read(int offset) -> std::uint8_t {
    auto& sweep = sequencer.sweep;
    auto& envelope = sequencer.envelope;
    
    switch (offset) {
      // Sweep Register
      case 0: {
        return sweep.shift |
              ((int)sweep.direction << 3) |
              (sweep.divider << 4);
      }
      case 1: return 0;
        
      // Duty/Len/Envelope
      case 2: {
        // Sound Length?
        return wave_duty << 6;
      }
      case 3: {
        return envelope.divider |
              ((int)envelope.direction << 3) |
              (envelope.initial_volume << 4);
      }
      
      // Frequency/Control
      case 4: return 0;
      case 5: {
        return length_enable ? 0x40 : 0;
      }
      
      default: return 0;
    }
  }
  
  void Write(int offset, std::uint8_t value) {
    auto& sweep = sequencer.sweep;
    auto& envelope = sequencer.envelope;
    
    switch (offset) {
      // Sweep Register
      case 0: {
        sweep.shift = value & 7;
        sweep.direction = Sweep::Direction((value >> 3) & 1);
        sweep.divider = value >> 4;
        break;
      }
      case 1: break;
        
      // Duty/Len/Envelope
      case 2: {
        length = value & 63;
        wave_duty = (value >> 6) & 3;
        break;
      }
      case 3: {
        envelope.divider = value & 7;
        envelope.direction = Envelope::Direction((value >> 3) & 1);
        envelope.initial_volume = value >> 4;
        break;
      }
      
      // Frequency Control
      case 4: {
        sweep.initial_freq = (sweep.initial_freq & ~0xFF) | value;
        break;
      }
      case 5: {
        sweep.initial_freq = (sweep.initial_freq & 0xFF) | (((int)value & 7) << 8);
        length_enable = value & 0x40;
        
        if (value & 0x80) {
          sequencer.Restart();
        }
        
        break;
      }
    }
  }
  
  // IO
  int  wave_duty;
  int  length;
  bool length_enable;
  
  std::int8_t sample = 0;
  int phase;
  Sequencer sequencer;
  
  Event event { 0, [this]() { this->Tick(); } };
};

}