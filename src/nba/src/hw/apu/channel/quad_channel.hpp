/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>
#include <nba/scheduler.hpp>

#include "hw/apu/channel/base_channel.hpp"

namespace nba::core {

class QuadChannel final : public BaseChannel {
public:
  QuadChannel(Scheduler& scheduler, Scheduler::EventClass event_class);

  void Reset();
  auto GetSample() -> s8 override { return sample; }
  void Generate();
  auto Read (int offset) -> u8;
  void Write(int offset, u8 value);

  void LoadState(SaveState::APU::IO::QuadChannel const& state);
  void CopyState(SaveState::APU::IO::QuadChannel& state);

private:
  static constexpr int GetSynthesisIntervalFromFrequency(int frequency) {
    // 128 cycles equals 131072 Hz, the highest possible frequency.
    // We are dividing by eight, because the waveform can change at
    // eight evenly spaced points inside a single cycle, depending on the wave duty.
    return 128 * (2048 - frequency) / 8;
  }

  Scheduler& scheduler;
  Scheduler::EventClass event_class;
  Scheduler::Event* event;

  s8 sample = 0;
  int phase;
  int wave_duty;
  bool dac_enable;
};

} // namespace nba::core
