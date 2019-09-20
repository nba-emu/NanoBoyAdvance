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
//#include "../../scheduler.hpp"

#include <cstdint>

namespace GameBoyAdvance {

class WaveChannel {

public:
  WaveChannel(Scheduler& scheduler) {
    sequencer.sweep.enabled = false;
    sequencer.envelope.enabled = false;
    
    scheduler.Add(event);
    scheduler.Add(sequencer.event);
    Reset();
  }
  
  void Reset() {
    sequencer.Reset();
    
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
    
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 16; j++) {
        wave_ram[i][j] = 0;
      }
    }
    
    event.countdown = GetSynthesisIntervalFromFrequency(0);
  }
  
  void Generate() {
    if (!playback || (apply_length && sequencer.length >= sound_length)) {
      sample = 0;
      event.countdown = GetSynthesisIntervalFromFrequency(0);
      return;
    }
    
    auto byte = wave_ram[bank_number][phase / 2];
    
    if ((phase % 2) == 0) {
      sample = byte >> 4;
    } else {
      sample = byte & 15;
    }

    constexpr int volume_table[4] = { 0, 4, 2, 1 };
    
    /* TODO: at 100% sample might overflow. */
    sample = (sample - 8) * 4 * (force_volume ? 3 : volume_table[volume]);
    
    if (++phase == 32) {
      phase = 0;
      
      if (dimension) {
        bank_number ^= 1;
      }
    }
    
    event.countdown += GetSynthesisIntervalFromFrequency(frequency);
  }
  
  auto Read(int offset) -> std::uint8_t {
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

        if (value & 0x80) {
          phase = 0;
          sequencer.Restart();
          
          // in 64-digit mode output starts with the first bank
          if (dimension) {
            bank_number = 0;
          }
        }
        break;
      }
    }
  }
  
  void WriteSample(int offset, std::uint8_t value) {
    wave_ram[bank_number ^ 1][offset] = value;
  }
  
  std::int8_t sample = 0;
  
private:
  constexpr int GetSynthesisIntervalFromFrequency(int frequency) {
    // 8 cycles equals 2097152 Hz, the highest possible sample rate.
    return 8 * (2048 - frequency);
  }
  
  Event event { 0, [this]() { this->Generate(); } };
  
  Sequencer sequencer;
  
  // TODO: rename this appropriately.
  bool playback;
  bool force_volume;
  bool apply_length;
  int volume;
  int frequency;
  int dimension;
  int bank_number;
  int sound_length;
  std::uint8_t wave_ram[2][16];
  
  int phase;
};

}