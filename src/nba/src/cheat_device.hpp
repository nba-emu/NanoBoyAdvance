/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include "bus/bus.hpp"
#include "scheduler.hpp"

namespace nba::core {

struct CheatDevice {
  CheatDevice(Bus& bus, Scheduler& scheduler);

  void Reset();

private:
  // Update around 20 times per second
  static constexpr int kCyclesPerUpdate = 838860;

  void Update(int late);
  void ExecuteARV3(u64* codes, size_t size);

  static auto DecryptARV3(u64 code) -> u64;

  Bus& bus;
  Scheduler& scheduler;
};

} // namespace nba::core
