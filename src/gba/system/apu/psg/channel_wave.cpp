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

#include "channel_wave.hpp"

using namespace GameBoyAdvance;

WaveChannel::WaveChannel(Scheduler& scheduler) {
  sequencer.sweep.enabled = false;
  sequencer.envelope.enabled = false;

  scheduler.Add(sequencer.event);
  scheduler.Add(event);
  Reset();
}

void WaveChannel::Reset() {
  sequencer.Reset();
  
  phase = 0;
  sample = 0;
  
  enabled = false;
  force_volume = false;
  volume = 0;
  frequency = 0;
  dimension = 0;
  wave_bank = 0;
  length_enable = false;
  length = 0;
  
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 16; j++) {
      wave_ram[i][j] = 0;
    }
  }
  
  event.countdown = GetSynthesisIntervalFromFrequency(0);
}

void WaveChannel::Generate() {
  if (!enabled || (length_enable && sequencer.length >= (256 - length))) {
    sample = 0;
    event.countdown = GetSynthesisIntervalFromFrequency(0);
    return;
  }
  
  auto byte = wave_ram[wave_bank][phase / 2];
  
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
      wave_bank ^= 1;
    }
  }
  
  event.countdown += GetSynthesisIntervalFromFrequency(frequency);
}

auto WaveChannel::Read(int offset) -> std::uint8_t {
  switch (offset) {
    /* Stop / Wave RAM select */
    case 0: {
      return (dimension << 5) |
             (wave_bank << 6) |
             (enabled ? 0x80 : 0);
    }
    case 1: return 0;

    /* Length / Volume */
    case 2: return 0;
    case 3: {
      return (volume << 5) |
             (force_volume  ? 0x80 : 0);
    }

    /* Frequency / Control */
    case 4: return 0;
    case 5: {
      return length_enable ? 0x40 : 0;
    }
              
    default: return 0;
  }
}

void WaveChannel::Write(int offset, std::uint8_t value) {
  switch (offset) {
    /* Stop / Wave RAM select */
    case 0: {
      dimension = (value >> 5) & 1;
      wave_bank = (value >> 6) & 1;
      enabled = value & 0x80;
      break;
    }
    case 1: break;

    /* Length / Volume */
    case 2: {
      length = value;
      break;
    }
    case 3: {
      volume = (value >> 5) & 3;
      force_volume = value & 0x80;
      break;
    }

    /* Frequency / Control */
    case 4: {
      frequency = (frequency & ~0xFF) | value;
      break;
    }
    case 5: {
      frequency = (frequency & 0xFF) | ((value & 7) << 8);
      length_enable = value & 0x40;

      if (value & 0x80) {
        phase = 0;
        sequencer.Restart();
        
        /* in 64-digit mode output starts with the first bank */
        if (dimension) {
          wave_bank = 0;
        }
      }
      break;
    }
  }
}
