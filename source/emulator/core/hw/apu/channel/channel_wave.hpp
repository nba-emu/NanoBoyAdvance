/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

#include "sequencer.hpp"

namespace nba::core {

class WaveChannel {
public:
  WaveChannel(Scheduler* scheduler);
  
  void Reset();
  
  void Generate();
  auto Read (int offset) -> std::uint8_t;
  void Write(int offset, std::uint8_t value);
  
  void WriteSample(int offset, std::uint8_t value) {
    wave_ram[wave_bank ^ 1][offset] = value;
  }
  
  std::int8_t sample = 0;
  
private:
  constexpr int GetSynthesisIntervalFromFrequency(int frequency) {
    // 8 cycles equals 2097152 Hz, the highest possible sample rate.
    return 8 * (2048 - frequency);
  }
  
  Scheduler::Event event { 0, [this] { this->Generate(); } };
  
  Sequencer sequencer;
  
  bool enabled;
  bool force_volume;
  int  volume;
  int  frequency;
  int  dimension;
  int  wave_bank;
  bool length_enable;
  
  std::uint8_t wave_ram[2][16];
  
  int phase;
};

} // namespace nba::core
