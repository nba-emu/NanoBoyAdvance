/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "ppu.hpp"

namespace nba::core {

void PPU::InitBackground() {
  const u64 timestamp_now = scheduler.GetTimestampNow();

  bg.timestamp_init = timestamp_now;
  bg.timestamp_last_sync = timestamp_now;
  bg.cycle = 0U;

  for(auto& text : bg.text) {
    text.fetching = false;
  }

  for(int id = 0; id < 2; id++) {
    bg.affine[id].x = mmio.bgx[id]._current;
    bg.affine[id].y = mmio.bgy[id]._current;
  }

  /**
   * We are latching this, because the currently the counter
   * in incremented in H-blank, which means that it might happen 
   * before BG rendering is done (because there is no sync point at H-blank).
   */
  bg.mosaic_counter = mmio.mosaic.bg._counter_y;
}

void PPU::DrawBackground() {
  const u64 timestamp_now = scheduler.GetTimestampNow();
  
  const int cycles = (int)(timestamp_now - bg.timestamp_last_sync);

  if(cycles == 0 || bg.cycle >= k_bg_cycle_limit) {
    return;
  }

  const int mode = mmio.dispcnt.mode;

  switch(mode) {
    case 0: DrawBackgroundImpl<0>(cycles); break;
    case 1: DrawBackgroundImpl<1>(cycles); break;
    case 2: DrawBackgroundImpl<2>(cycles); break;
    case 3: DrawBackgroundImpl<3>(cycles); break;
    case 4: DrawBackgroundImpl<4>(cycles); break;
    case 5: DrawBackgroundImpl<5>(cycles); break;
    case 6: 
    case 7: DrawBackgroundImpl<7>(cycles); break;
  }

  bg.timestamp_last_sync = timestamp_now;
}

template<int mode> void PPU::DrawBackgroundImpl(int cycles) {
  /**
   * @todo: we are losing out on some possible optimizations,
   * by implementing the various BG modes in separate methods,
   * which we have to call on a per-cycle basis.
   */
  for(int i = 0; i < cycles; i++) {
    // We add one to the cycle counter for convenience,
    // because it makes some of the timing math simpler.
    const uint cycle = 1U + bg.cycle;

    // text-mode backgrounds
    if constexpr(mode <= 1) {
      const uint id = cycle & 3U; // BG0 - BG3

      if((id <= 1 || mode == 0) && mmio.enable_bg[0][id]) {
        RenderMode0BG(id, cycle);
      }
    }

    // affine backgrounds
    if constexpr(mode == 1 || mode == 2) {
      const int id = ~(cycle >> 1) & 1; // 0: BG2, 1: BG3

      if((id == 0 || mode == 2) && mmio.enable_bg[0][2 + id]) {
        RenderMode2BG(id, cycle);
      }
    }

    if constexpr(mode == 3) {
      if(mmio.enable_bg[0][2]) {
        RenderMode3BG(cycle);
      }
    }

    if constexpr(mode == 4) {
      if(mmio.enable_bg[0][2]) {
        RenderMode4BG(cycle);
      }
    }

    if constexpr(mode == 5) {
      if(mmio.enable_bg[0][2]) {
        RenderMode5BG(cycle);
      }
    }

    // @todo: I don't think this is always correct, at least in text-mode.
    if(++bg.cycle == k_bg_cycle_limit) {
      break;
    }
  }
}

} // namespace nba::core
