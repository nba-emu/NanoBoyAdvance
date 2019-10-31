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

void PPU::RenderLayerAffine(int id) {
  auto const& bg = mmio.bgcnt[2 + id];
  
  std::uint32_t tile_base = bg.tile_block * 16384;
  
  std::int32_t ref_x = mmio.bgx[id]._current;
  std::int32_t ref_y = mmio.bgy[id]._current;
  std::int16_t pa = mmio.bgpa[id];
  std::int16_t pc = mmio.bgpc[id];
  
  int size;
  int block_width;
  
  switch (bg.size) {
    case 0: size = 128;  block_width = 16;  break;
    case 1: size = 256;  block_width = 32;  break;
    case 2: size = 512;  block_width = 64;  break;
    case 3: size = 1024; block_width = 128; break;
  }
  
  for (int _x = 0; _x < 240; _x++) {
    bool is_backdrop = false;
    
    std::int32_t x = ref_x >> 8;
    std::int32_t y = ref_y >> 8;
    
    ref_x += pa;
    ref_y += pc;
    
    if (x >= size) {
      if (bg.wraparound) {
        x = x % size;
      } else {
        is_backdrop = true;
      }
    } else if (x < 0) {
      if (bg.wraparound) {
        //x = (size + x) % size;
        x = size + (x % size);
      } else {
        is_backdrop = true;
      }
    }

    if (y >= size) {
      if (bg.wraparound) {
        y = y % size;
      } else {
        is_backdrop = true;
      }
    } else if (y < 0) {
      if (bg.wraparound) {
        //y = (size + y) % size;
        y = size + (y % size);
      } else {
        is_backdrop = true;
      }
    }

    if (is_backdrop) {
      DrawPixel(_x, 2 + id, bg.priority, s_color_transparent);
      continue;
    }
    
    int map_x  = x / 8;
    int map_y  = y / 8;
    int tile_x = x % 8;
    int tile_y = y % 8;

    int number = vram[(bg.map_block << 11) + map_y * block_width + map_x];
    
    DrawPixel(_x, 2 + id, bg.priority, DecodeTilePixel8BPP(tile_base, number, tile_x, tile_y));
  }
}