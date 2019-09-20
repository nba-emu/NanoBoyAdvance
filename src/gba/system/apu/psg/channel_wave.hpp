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

//#include "sequencer.hpp"
#include "../../scheduler.hpp"

#include <cstdint>

namespace GameBoyAdvance {

class WaveChannel {

public:
  WaveChannel(Scheduler& scheduler) {
    scheduler.Add(event);
    Reset();
  }
  
  void Reset() {
    phase = 0;
    sample = 0;
    
    playback = false;
    force_volume = false;
    apply_length = false;
    volume = 0;
    frequency = 0;
    dimension = 0;
    bank_number = 0;
    sound_length = 0;
    
    event.countdown = GetSynthesisIntervalFromFrequency(0);
  }
  
  void Generate() {
    // TODO: this is just a test.
    if (phase < 8) {
      sample = 127;
    } else {
      sample = -128;
    }
    
    phase = (phase + 1) % 16;
    
    event.countdown += GetSynthesisIntervalFromFrequency(frequency);
  }
  
  auto Read (int offset) -> std::uint8_t {
    switch (offset) {
      /* Stop / Wave RAM select */
      case 0: {
        return (dimension   << 5       ) |
               (bank_number << 6       ) |
               (playback     ? 0x80 : 0);
      }
      case 1: return 0;

      /* Length / Volume */
      case 2: return 0;
      case 3: {
        return (volume       << 5       ) |
               (force_volume  ? 0x80 : 0);
      }

      /* Frequency / Control */
      case 4: return 0;
      case 5: {
        return apply_length ? 0x40 : 0;
      }
                
      default: return 0;
    }
  }
  
  void Write(int offset, std::uint8_t value) {
    switch (offset) {
      /* Stop / Wave RAM select */
      case 0: {
        dimension   = (value >> 5) & 1;
        bank_number = (value >> 6) & 1;
        playback    =  value & 0x80;
        break;
      }
      case 1: break;

      /* Length / Volume */
      case 2: {
        sound_length = value;
        break;
      }
      case 3: {
        volume       = (value >> 5) & 3;
        force_volume = value & 0x80;
        break;
      }

      /* Frequency / Control */
      case 4: {
        frequency = (frequency & ~0xFF) | value;
        break;
      }
      case 5: {
        frequency    = (frequency & 0xFF) | ((value & 7) << 8);
        apply_length = value & 0x40;

        // on sound restart
        if (value & 0x80) {
          // TODO:
          //internal.length_cycles = 0;
          
          // in 64-digit mode output starts with the first bank
          if (dimension) {
            bank_number = 0;
          }
        }
        break;
      }
    }
  }
  
  std::int8_t sample = 0;
  
private:
  constexpr int GetSynthesisIntervalFromFrequency(int frequency) {
    // 128 cycles equals 131072 Hz, the highest possible frequency.
    // We are dividing by 16, because the waveform can change at
    // eight evenly spaced points inside a single cycle, depending on the wave ram pattern.
    return 128 * (2048 - frequency) / 16;
  }
  
  Event event { 0, [this]() { this->Generate(); } };
  
  //Sequencer sequencer;
  
  // TODO: rename this appropriately.
  bool playback;
  bool force_volume;
  bool apply_length;
  int volume;
  int frequency;
  int dimension;
  int bank_number;
  int sound_length;
  
  int phase;
};

}