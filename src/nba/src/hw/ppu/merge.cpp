/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "ppu.hpp"

namespace nba::core {

void PPU::InitMerge() {
  merge.timestamp_last_sync = scheduler.GetTimestampNow();
  merge.cycle = 0U;
}

void PPU::DrawMerge() {
  const u64 timestamp_now = scheduler.GetTimestampNow();
  
  int cycles = (int)(timestamp_now - merge.timestamp_last_sync);

  if(cycles == 0 || merge.cycle >= k_bg_cycle_limit) {
    return;
  }

  fmt::print("PPU: draw scanline @ VCOUNT={} delta={}\n", mmio.vcount, cycles);

  merge.timestamp_last_sync = timestamp_now;
}

} // namespace nba::core
