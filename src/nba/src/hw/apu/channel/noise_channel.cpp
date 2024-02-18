/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "hw/apu/channel/noise_channel.hpp"
#include "hw/apu/registers.hpp"

namespace nba::core {

NoiseChannel::NoiseChannel(Scheduler& scheduler, BIAS& bias)
    : BaseChannel(true, false)
    , scheduler(scheduler)
    , bias(bias) {
  scheduler.Register(Scheduler::EventClass::APU_PSG4_generate, this, &NoiseChannel::Generate);
  
  Reset();
}

void NoiseChannel::Reset() {
  BaseChannel::Reset();

  frequency_shift = 0;
  frequency_ratio = 0;
  width = 0;
  dac_enable = false;

  lfsr = 0;
  sample = 0;
  skip_count = 0;

  event = nullptr;
}

void NoiseChannel::Generate() {
  if(!IsEnabled()) {
    sample = 0;
    event = nullptr;
    return;
  }

  static constexpr u16 lfsr_xor[2] = { 0x6000, 0x60 };

  int carry = lfsr & 1;

  lfsr >>= 1;
  if(carry) {
    lfsr ^= lfsr_xor[width];
    sample = +8;
  } else {
    sample = -8;
  }

  sample *= envelope.current_volume;

  if(!dac_enable) sample = 0;

  // Skip samples that will never be sampled by the audio mixer.
  for(int i = 0; i < skip_count; i++) {
    carry = lfsr & 1;
    lfsr >>= 1;
    if(carry) {
      lfsr ^= lfsr_xor[width];
    }
  }

  int noise_interval = GetSynthesisInterval(frequency_ratio, frequency_shift);
  int mixer_interval = bias.GetSampleInterval();

  /* If a channel generates at a higher rate than
   * the audio mixer samples it, then it will generate samples
   * that will be skipped anyways.
   * In that case we can sample the channel at the same rate
   * as the mixer rate and only output the sample that will be used.
   */
  if(noise_interval < mixer_interval) {
    skip_count = mixer_interval/noise_interval - 1;
    noise_interval = mixer_interval;
  } else {
    skip_count = 0;
  }

  event = scheduler.Add(noise_interval, Scheduler::EventClass::APU_PSG4_generate);
}

auto NoiseChannel::Read(int offset) -> u8 {
  switch(offset) {
    // Length / Envelope
    case 0: return 0;
    case 1: {
      return envelope.divider |
            ((int)envelope.direction << 3) |
            (envelope.initial_volume << 4);
    }
    case 2:
    case 3: return 0;

    // Frequency / Control
    case 4: {
      return frequency_ratio |
            (width << 3) |
            (frequency_shift << 4);
    }
    case 5: {
      return length.enabled ? 0x40 : 0;
    }

    default: return 0;
  }
}

void NoiseChannel::Write(int offset, u8 value) {
  switch(offset) {
    // Length / Envelope
    case 0: {
      length.length = 64 - (value & 63);
      break;
    }
    case 1: {
      envelope.divider = value & 7;
      envelope.direction = Envelope::Direction((value >> 3) & 1);
      envelope.initial_volume = value >> 4;

      dac_enable = (value >> 3) != 0;
      if(!dac_enable) {
        Disable();
      }
      break;
    }

    // Frequency / Control
    case 4: {
      frequency_ratio = value & 7;
      width = (value >> 3) & 1;
      frequency_shift = value >> 4;
      break;
    }
    case 5: {
      length.enabled = value & 0x40;

      if(dac_enable && (value & 0x80)) {
        if(!IsEnabled()) {
          skip_count = 0;
          if(event) {
            scheduler.Cancel(event);
          }
          event = scheduler.Add(GetSynthesisInterval(frequency_ratio, frequency_shift), Scheduler::EventClass::APU_PSG4_generate);
        }

        static constexpr u16 lfsr_init[] = { 0x4000, 0x0040 };
        lfsr = lfsr_init[width];
        Restart();
      }
      break;
    }
  }
}

} // namespace nba::core
