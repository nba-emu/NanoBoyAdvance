/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "../ppu.hpp"

using namespace GameBoyAdvance;

void PPU::RenderWindow(int id) {
  int line = mmio.vcount;
  auto& winv = mmio.winv[id];

  /* Check if the current scanline is outside of the window. */
  if ((winv.min <= winv.max && (line < winv.min || line >= winv.max)) ||
      (winv.min >  winv.max && (line < winv.min && line >= winv.max))) {
    /* Mark window as inactive during the current scanline. */
    window_scanline_enable[id] = false;
  } else {
    auto& winh = mmio.winh[id];

    /* Mark window as active during the current scanline. */
    window_scanline_enable[id] = true;

    /* Only recalculate the LUTs if min/max changed between the last update & now. */
    if (winh._changed) {
      if (winh.min <= winh.max) {
        for (int x = 0; x < 240; x++) {
          buffer_win[id][x] = x >= winh.min && x < winh.max;
        }
      } else {
        for (int x = 0; x < 240; x++) {
          buffer_win[id][x] = x >= winh.min || x < winh.max;
        }
      }
      winh._changed = false;
    }
  }
}