/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "channel_noise.hpp"

namespace nba::core {

NoiseChannel::NoiseChannel(Scheduler& scheduler, BIAS& bias)
    : scheduler(scheduler)
    , sequencer(scheduler)
    , bias(bias) {
  sequencer.sweep.enabled = false;
  sequencer.envelope.enabled = true;
  Reset();
}

void NoiseChannel::Reset() {
  sequencer.Reset();

  frequency_shift = 0;
  frequency_ratio = 0;
  width = 0;
  length_enable = false;

  lfsr = 0;
  sample = 0;
  skip_count = 0;

  scheduler.Add(GetSynthesisInterval(7, 15), event_cb);
}

void NoiseChannel::Generate(int cycles_late) {
  if (length_enable && sequencer.length <= 0) {
    sample = 0;
    scheduler.Add(GetSynthesisInterval(7, 15) - cycles_late, event_cb);
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

  /* Skip samples that will never be sampled by the audio mixer. */
  for (int i = 0; i < skip_count; i++) {
    carry = lfsr & 1;
    lfsr >>= 1;
    if (carry) {
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
  if (noise_interval < mixer_interval) {
    skip_count = mixer_interval/noise_interval - 1;
    noise_interval = mixer_interval;
  } else {
    skip_count = 0;
  }

  scheduler.Add(noise_interval - cycles_late, event_cb);
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
      sequencer.length = 64 - (value & 63);
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
        /* TODO: are these the correct initialization values? */
        const std::uint16_t lfsr_init[] = { 0x4000, 0x0040 };

        sequencer.Restart();
        lfsr = lfsr_init[width];
      }
      break;
    }
  }
}

} // namespace nba::core
