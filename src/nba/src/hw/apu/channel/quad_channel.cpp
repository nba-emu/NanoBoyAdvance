/*
 * Copyright (C) 2024 fleroviux
 *<<
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "hw/apu/channel/quad_channel.hpp"

namespace nba::core {

QuadChannel::QuadChannel(Scheduler& scheduler, Scheduler::EventClass event_class)
    : BaseChannel(true, true)
    , scheduler(scheduler)
    , event_class(event_class) {
  scheduler.Register(event_class, this, &QuadChannel::Generate);

  Reset();
}

void QuadChannel::Reset() {
  BaseChannel::Reset();
  phase = 0;
  sample = 0;
  wave_duty = 0;
  dac_enable = false;
  event = nullptr;
}

void QuadChannel::Generate() {
  if(!IsEnabled()) {
    sample = 0;
    event = nullptr;
    return;
  }

  static constexpr int pattern[4][8] = {
    { +8, -8, -8, -8, -8, -8, -8, -8 },
    { +8, +8, -8, -8, -8, -8, -8, -8 },
    { +8, +8, +8, +8, -8, -8, -8, -8 },
    { +8, +8, +8, +8, +8, +8, -8, -8 }
  };

  if(dac_enable) {
    sample = s8(pattern[wave_duty][phase] * envelope.current_volume);
  } else {
    sample = 0;
  }
  phase = (phase + 1) % 8;

  event = scheduler.Add(GetSynthesisIntervalFromFrequency(sweep.current_freq), event_class);
}

auto QuadChannel::Read(int offset) -> u8 {
  switch(offset) {
    // Sweep Register
    case 0: {
      return sweep.shift |
            ((int)sweep.direction << 3) |
            (sweep.divider << 4);
    }
    case 1: return 0;

    // Wave Duty / Length / Envelope
    case 2: {
      return wave_duty << 6;
    }
    case 3: {
      return envelope.divider |
            ((int)envelope.direction << 3) |
            (envelope.initial_volume << 4);
    }

    // Frequency / Control
    case 4: return 0;
    case 5: {
      return length.enabled ? 0x40 : 0;
    }

    default: return 0;
  }
}

void QuadChannel::Write(int offset, u8 value) {
  switch(offset) {
    // Sweep Register
    case 0: {
      sweep.shift = value & 7;
      sweep.direction = Sweep::Direction((value >> 3) & 1);
      sweep.divider = (value >> 4) & 7;
      break;
    }
    case 1: break;

    // Wave Duty / Length / Envelope
    case 2: {
      length.length = 64 - (value & 63);
      wave_duty = (value >> 6) & 3;
      break;
    }
    case 3: {
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
      sweep.initial_freq = (sweep.initial_freq & ~0xFF) | value;
      sweep.current_freq = sweep.initial_freq;
      break;
    }
    case 5: {
      sweep.initial_freq = (sweep.initial_freq & 0xFF) | (((int)value & 7) << 8);
      sweep.current_freq = sweep.initial_freq;
      length.enabled = value & 0x40;

      if(dac_enable && (value & 0x80)) {
        if(!IsEnabled()) {
          if(event) {
            scheduler.Cancel(event);
          }
          event = scheduler.Add(GetSynthesisIntervalFromFrequency(sweep.current_freq), event_class);
        }
        phase = 0;
        Restart();
      }

      break;
    }
  }
}

} // namespace nba::core
