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

void PPU::RenderLayerText(int id) {
  auto const& bgcnt  = mmio.bgcnt[id];
  auto const& mosaic = mmio.mosaic.bg;
  
  std::uint32_t tile_base = bgcnt.tile_block * 16384;
   
  int line = mmio.bgvofs[id] + mmio.vcount;
  
  /* Apply vertical mosaic */
  if (bgcnt.mosaic_enable) {
    line -= mosaic._counter_y;
  }

  int draw_x = -(mmio.bghofs[id] % 8);
  int grid_x =   mmio.bghofs[id] / 8;
  int grid_y = line / 8;
  int tile_y = line % 8;
  
  int screen_x;
  int screen_y = (grid_y / 32) % 2;

  std::uint16_t tile[8];
  std::int32_t  last_encoder = -1;
  std::uint16_t encoder;
  std::uint32_t offset;
  std::uint32_t base = (bgcnt.map_block * 2048) + ((grid_y % 32) * 64);

  std::uint16_t* buffer = buffer_bg[id];
  
  while (draw_x < 240) {
    screen_x = (grid_x / 32) % 2;
    offset = base + ((grid_x++ % 32) * 2);

    switch (bgcnt.size) {
      case 1: offset +=  screen_x * 2048; break;
      case 2: offset +=  screen_y * 2048; break;
      case 3: offset += (screen_x * 2048) + (screen_y * 4096); break;
    }

    encoder = (vram[offset + 1] << 8) | vram[offset];

    if (encoder != last_encoder) {
      int number  = encoder & 0x3FF;
      int palette = encoder >> 12;
      bool flip_x = encoder & (1 << 10);
      bool flip_y = encoder & (1 << 11);
      int _tile_y = flip_y ? (tile_y ^ 7) : tile_y;

      if (!bgcnt.full_palette) {
        DecodeTileLine4BPP(tile, tile_base, palette, number, _tile_y, flip_x);
      } else {
        DecodeTileLine8BPP(tile, tile_base, number, _tile_y, flip_x);
      }

      last_encoder = encoder;
    }

    if (draw_x >= 0 && draw_x <= 232) {
      for (int x = 0; x < 8; x++) {
        buffer[draw_x++] = tile[x];
      }
    } else {
      int x = 0;
      int max = 8;

      if (draw_x < 0) {
        x = -draw_x;
        draw_x = 0;
      }
      if (draw_x > 232) {
        max -= draw_x - 232;
      }

      for (x; x < max; x++) {
        buffer[draw_x++] = tile[x];
      }
    }
  }

  /* Apply horizontal works. */
  if (bgcnt.mosaic_enable && mosaic.size_x != 1) {
    int mosaic_x = 0;

    for (int x = 0; x < 240; x++) {
      buffer[x] = buffer[x - mosaic_x];
      if (++mosaic_x == mosaic.size_x) {
        mosaic_x = 0;
      }
    }
  }
}
