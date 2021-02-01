/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>
#include <emulator/core/scheduler.hpp>

#include "sequencer.hpp"

namespace nba::core {

class QuadChannel : public BaseChannel {
public:
  QuadChannel(Scheduler& scheduler);

  void Reset();
  void Generate(int cycles_late);
  auto Read (int offset) -> std::uint8_t;
  void Write(int offset, std::uint8_t value);

  std::int8_t sample = 0;

private:
  constexpr int GetSynthesisIntervalFromFrequency(int frequency) {
    // 128 cycles equals 131072 Hz, the highest possible frequency.
    // We are dividing by eight, because the waveform can change at
    // eight evenly spaced points inside a single cycle, depending on the wave duty.
    return 128 * (2048 - frequency) / 8;
  }

  Scheduler& scheduler;
  std::function<void(int)> event_cb = [this](int cycles_late) {
    this->Generate(cycles_late);
  };

  int phase;
  int wave_duty;
  bool dac_enable;
};

} // namespace nba::core
