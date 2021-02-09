/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "quad_channel.hpp"

namespace nba::core {

QuadChannel::QuadChannel(Scheduler& scheduler)
    : BaseChannel(true, true)
    , scheduler(scheduler) {
  Reset();
}

void QuadChannel::Reset() {
  BaseChannel::Reset();
  phase = 0;
  sample = 0;
  wave_duty = 0;
  dac_enable = false;
}

void QuadChannel::Generate(int cycles_late) {
  if (!IsEnabled()) {
    sample = 0;
    return;
  }

  constexpr std::int16_t pattern[4][8] = {
    { +8, -8, -8, -8, -8, -8, -8, -8 },
    { +8, +8, -8, -8, -8, -8, -8, -8 },
    { +8, +8, +8, +8, -8, -8, -8, -8 },
    { +8, +8, +8, +8, +8, +8, -8, -8 }
  };

  if (dac_enable) {
    sample = std::int8_t(pattern[wave_duty][phase] * envelope.current_volume);
  } else {
    sample = 0;
  }
  phase = (phase + 1) % 8;

  scheduler.Add(GetSynthesisIntervalFromFrequency(sweep.current_freq) - cycles_late, event_cb);
}

auto QuadChannel::Read(int offset) -> std::uint8_t {
  switch (offset) {
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

void QuadChannel::Write(int offset, std::uint8_t value) {
  switch (offset) {
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
      auto divider_old = envelope.divider;
      auto direction_old = envelope.direction;

      envelope.divider = value & 7;
      envelope.direction = Envelope::Direction((value >> 3) & 1);
      envelope.initial_volume = value >> 4;

      dac_enable = (value >> 3) != 0;
      if (!dac_enable) {
        Disable();
      }

      // Handle envelope "Zombie" mode:
      // https://gist.github.com/drhelius/3652407#file-game-boy-sound-operation-L491
      // TODO: what is the exact behavior on AGB systems?
      if (divider_old == 0 && envelope.active) {
        envelope.current_volume++;
      } else if (direction_old == Envelope::Direction::Decrement) {
        envelope.current_volume += 2;
      }
      if (direction_old != envelope.direction) {
        envelope.current_volume = 16 - envelope.current_volume;
      }
      envelope.current_volume &= 15;
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

      if (dac_enable && (value & 0x80)) {
        if (!IsEnabled()) {
          // TODO: properly align event to system clock.
          scheduler.Add(GetSynthesisIntervalFromFrequency(sweep.current_freq), event_cb);
        }
        phase = 0;
        Restart();
      }

      break;
    }
  }
}

} // namespace nba::core
