/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
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