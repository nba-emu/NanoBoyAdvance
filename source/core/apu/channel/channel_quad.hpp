/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include "sequencer.hpp"

#include <cstdint>

namespace GameBoyAdvance {

class QuadChannel {
public:
  QuadChannel(Scheduler& scheduler);
    
  void Reset();
  
  void Generate();
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

  Event event { 0, [this]() { this->Generate(); } };
  Sequencer sequencer;
  int phase;
  int wave_duty;
  bool length_enable;
};

}