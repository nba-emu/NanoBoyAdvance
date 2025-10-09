/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "ppu.hpp"

namespace nba::core {

void PPU::InitWindow() {
  const int vcount = mmio.vcount;

  for(int i = 0; i < 2; i++) {
    const auto& winv = mmio.winv[i];

    if(vcount == winv.min) {
      window.v_flag[i] = true;
    }

    if(vcount == winv.max) {
      window.v_flag[i] = false;
    }
  }

  window.timestamp_last_sync = scheduler.GetTimestampNow();
  window.cycle = 0;
}

void PPU::DrawWindow() {
  const u64 timestamp_now = scheduler.GetTimestampNow();

  const int cycles = (int)(timestamp_now - window.timestamp_last_sync);

  if(cycles == 0 || window.cycle >= 1024U) {
    return;
  }

  for(int i = 0; i < cycles; i++) {
    if((window.cycle & 3U) == 0U) {
      const uint x = window.cycle >> 2;

      for(int i = 0; i < 2; i++) {
        const auto& winh = mmio.winh[i];

        if(x == winh.min) {
          window.h_flag[i] = true;
        }

        if(x == winh.max) {
          window.h_flag[i] = false;
        }

        if(x < 240) {
          window.buffer[x][i] = window.h_flag[i] && window.v_flag[i];
        }
      }
    }

    if(++window.cycle == 1024U) {
      break;
    }
  }

  window.timestamp_last_sync = timestamp_now;
}

} // namespace nba::core
