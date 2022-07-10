/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>

#include "hw/apu/channel/base_channel.hpp"
#include "scheduler.hpp"

namespace nba::core {

class QuadChannel : public BaseChannel {
public:
  QuadChannel(Scheduler& scheduler);

  void Reset();
  auto GetSample() -> s8 override { return sample; }
  void Generate(int cycles_late);
  auto Read (int offset) -> u8;
  void Write(int offset, u8 value);

private:
  static constexpr int GetSynthesisIntervalFromFrequency(int frequency) {
    // 128 cycles equals 131072 Hz, the highest possible frequency.
    // We are dividing by eight, because the waveform can change at
    // eight evenly spaced points inside a single cycle, depending on the wave duty.
    return 128 * (2048 - frequency) / 8;
  }

  Scheduler& scheduler;
  std::function<void(int)> event_cb = [this](int cycles_late) {
    this->Generate(cycles_late);
  };

  s8 sample = 0;
  int phase;
  int wave_duty;
  bool dac_enable;
};

} // namespace nba::core
