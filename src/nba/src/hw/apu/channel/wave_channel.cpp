/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "hw/apu/channel/wave_channel.hpp"

namespace nba::core {

WaveChannel::WaveChannel(Scheduler& scheduler)
    : BaseChannel(false, false, 256)
    , scheduler(scheduler) {
  scheduler.Register(Scheduler::EventClass::APU_PSG3_generate, this, &WaveChannel::Generate);

  Reset(WaveChannel::ResetWaveRAM::Yes);
}

void WaveChannel::Reset(ResetWaveRAM reset_wave_ram) {
  BaseChannel::Reset();

  phase = 0;
  sample = 0;

  playing = false;
  force_volume = false;
  volume = 0;
  frequency = 0;
  dimension = 0;
  wave_bank = 0;

  if(reset_wave_ram == ResetWaveRAM::Yes) {
    for(int i = 0; i < 2; i++) {
      for(int j = 0; j < 16; j++) {
        wave_ram[i][j] = 0;
      }
    }
  }

  event = nullptr;
}

void WaveChannel::Generate() {
  if(!IsEnabled()) {
    sample = 0;
    if(BaseChannel::IsEnabled()) {
      event = scheduler.Add(GetSynthesisIntervalFromFrequency(frequency), Scheduler::EventClass::APU_PSG3_generate);
    } else {
      event = nullptr;
    }
    return;
  }

  auto byte = wave_ram[wave_bank][phase / 2];

  if((phase % 2) == 0) {
    sample = byte >> 4;
  } else {
    sample = byte & 15;
  }

  constexpr int volume_table[4] = { 0, 4, 2, 1 };

  sample = (sample - 8) * 4 * (force_volume ? 3 : volume_table[volume]);

  if(++phase == 32) {
    phase = 0;
    if(dimension) {
      wave_bank ^= 1;
    }
  }

  event = scheduler.Add(GetSynthesisIntervalFromFrequency(frequency), Scheduler::EventClass::APU_PSG3_generate);
}

auto WaveChannel::Read(int offset) -> u8 {
  switch(offset) {
    // Stop / Wave RAM select
    case 0: {
      return (dimension << 5) |
             (wave_bank << 6) |
             (playing ? 0x80 : 0);
    }
    case 1: return 0;

    // Length / Volume
    case 2: return 0;
    case 3: {
      return (volume << 5) |
             (force_volume  ? 0x80 : 0);
    }

    // Frequency / Control
    case 4: return 0;
    case 5: {
      return length.enabled ? 0x40 : 0;
    }

    default: return 0;
  }
}

void WaveChannel::Write(int offset, u8 value) {
  switch(offset) {
    // Stop / Wave RAM select
    case 0: {
      dimension = (value >> 5) & 1;
      wave_bank = (value >> 6) & 1;
      playing = value & 0x80;
      break;
    }
    case 1: break;

    // Length / Volume
    case 2: {
      length.length = 256 - value;
      break;
    }
    case 3: {
      volume = (value >> 5) & 3;
      force_volume = value & 0x80;
      break;
    }

    // Frequency / Control
    case 4: {
      frequency = (frequency & ~0xFF) | value;
      break;
    }
    case 5: {
      frequency = (frequency & 0xFF) | ((value & 7) << 8);
      length.enabled = value & 0x40;

      if(playing && (value & 0x80)) {
        if(!BaseChannel::IsEnabled()) {
          if(event) {
            scheduler.Cancel(event);
          }
          event = scheduler.Add(GetSynthesisIntervalFromFrequency(frequency), Scheduler::EventClass::APU_PSG3_generate);
        }
        phase = 0;
        if(dimension) {
          wave_bank = 0;
        }
        Restart();
      }
      break;
    }
  }
}

} // namespace nba::core
