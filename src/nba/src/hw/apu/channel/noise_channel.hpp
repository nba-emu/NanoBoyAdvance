/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>

#include "hw/apu/channel/base_channel.hpp"
#include "scheduler.hpp"

namespace nba::core {

struct BIAS;

class NoiseChannel : public BaseChannel {
public:
  NoiseChannel(Scheduler& scheduler, BIAS& bias);

  void Reset();
  auto GetSample() -> s8 override { return sample; }
  void Generate();
  auto Read (int offset) -> u8;
  void Write(int offset, u8 value);

  void LoadState(SaveState::APU::IO::NoiseChannel const& state);
  void CopyState(SaveState::APU::IO::NoiseChannel& state);

private:
  static constexpr int GetSynthesisInterval(int ratio, int shift) {
    int interval = 64 << shift;

    if (ratio == 0) {
      interval /= 2;
    } else {
      interval *= ratio;
    }

    return interval;
  }

  u16 lfsr;
  s8 sample = 0;

  Scheduler& scheduler;
  Scheduler::Event* event;

  int frequency_shift;
  int frequency_ratio;
  int width;
  bool dac_enable;

  BIAS& bias;
  int skip_count;
};

} // namespace nba::core
