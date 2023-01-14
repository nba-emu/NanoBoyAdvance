/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "ppu.hpp"

namespace nba::core {

void PPU::InitBackground() {
  bg.timestamp_last_sync = scheduler.GetTimestampNow();
}

void PPU::DrawBackground() {
  u64 timestamp_now = scheduler.GetTimestampNow();

  if(timestamp_now == bg.timestamp_last_sync) {
    return;
  }

  fmt::print("PPU: draw background @ VCOUNT={} delta={}\n", mmio.vcount, timestamp_now - bg.timestamp_last_sync);

  bg.timestamp_last_sync = timestamp_now;
}

} // namespace nba::core
