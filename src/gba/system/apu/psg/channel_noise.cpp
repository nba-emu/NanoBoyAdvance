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

#include "channel_noise.hpp"

using namespace GameBoyAdvance;

NoiseChannel::NoiseChannel(Scheduler& scheduler) {
  sequencer.sweep.enabled = false;
  sequencer.envelope.enabled = true;

  scheduler.Add(event);
  scheduler.Add(sequencer.event);
  Reset();
}

void NoiseChannel::Reset() {
  sequencer.Reset();
  
  frequency_shift = 0;
  frequency_ratio = 0;
  width = 0;
  length = 0;
  length_enable = false;
  
  lfsr = 0;
  sample = 0;
  
  event.countdown = GetSynthesisInterval(7, 15);
}

void NoiseChannel::Generate() {
  if (length_enable && sequencer.length >= (64 - length)) {
    sample = 0;
    event.countdown = GetSynthesisInterval(7, 15);
    return;
  }

  constexpr std::uint16_t lfsr_xor[2] = { 0x6000, 0x60 };
  
  int carry = lfsr & 1;
  
  lfsr >>= 1;
  if (carry) {
    lfsr ^= lfsr_xor[width];
    sample = +8;
  } else {
    sample = -8;
  }
  
  sample *= sequencer.envelope.current_volume;
  
  event.countdown += GetSynthesisInterval(frequency_ratio, frequency_shift);
}

auto NoiseChannel::Read(int offset) -> std::uint8_t {
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
      return frequency_ratio |
            (width << 3) |
            (frequency_shift << 4);
    }
    case 5: {
      return length_enable ? 0x40 : 0;
    }
              
    default: return 0;
  }
}

void NoiseChannel::Write(int offset, std::uint8_t value) {
  auto& envelope = sequencer.envelope;
  
  switch (offset) {
    /* Length / Envelope */
    case 0: {
      length = value & 63;
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
      frequency_ratio = value & 7;
      width = (value >> 3) & 1;
      frequency_shift = value >> 4;
      break;
    }
    case 5: {
      length_enable = value & 0x40;

      if (value & 0x80) {
        const std::uint16_t lfsr_init[] = { 0x4000, 0x0040 };
        
        sequencer.Restart();
        lfsr = lfsr_init[width];
      }
      break;
    }
  }
}
