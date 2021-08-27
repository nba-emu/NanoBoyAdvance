/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>
#include <emulator/core/scheduler.hpp>

#include "base_channel.hpp"

namespace nba::core {

class WaveChannel : public BaseChannel {
public:
  WaveChannel(Scheduler& scheduler);

  void Reset();
  bool IsEnabled() override { return playing && BaseChannel::IsEnabled(); }
  auto GetSample() -> s8 override { return sample; }
  void Generate(int cycles_late);
  auto Read (int offset) -> u8;
  void Write(int offset, u8 value);

  auto ReadSample(int offset) -> u8 {
    return wave_ram[wave_bank ^ 1][offset];
  }

  void WriteSample(int offset, u8 value) {
    wave_ram[wave_bank ^ 1][offset] = value;
  }

private:
  constexpr int GetSynthesisIntervalFromFrequency(int frequency) {
    // 8 cycles equals 2097152 Hz, the highest possible sample rate.
    return 8 * (2048 - frequency);
  }

  Scheduler& scheduler;
  std::function<void(int)> event_cb = [this](int cycles_late) {
    this->Generate(cycles_late);
  };

  s8 sample = 0;
  bool playing;
  bool force_volume;
  int volume;
  int frequency;
  int dimension;
  int wave_bank;
  u8 wave_ram[2][16];
  int phase;
};

} // namespace nba::core
