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
  auto const& mosaic = mmio.mosaic.bg;
  
  std::uint32_t tile_base = bg.tile_block * 16384;
  std::uint32_t map_base = bg.map_block * 2048;
  
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
  
  std::uint16_t* buffer = buffer_bg[2 + id];
  
  int mosaic_x = 0;
  
  for (int _x = 0; _x < 240; _x++) {
    std::int32_t x = ref_x >> 8;
    std::int32_t y = ref_y >> 8;
    
    if (bg.mosaic_enable) {
      if (++mosaic_x == mosaic.size_x) {
        ref_x += mosaic.size_x * pa;
        ref_y += mosaic.size_x * pc;
        mosaic_x = 0;
      }
    } else {
      ref_x += pa;
      ref_y += pc;
    }
    
    if (bg.wraparound) {
      if (x >= size) {
        x %= size;
      } else if (x < 0) {
        x = size + (x % size);
      }
      
      if (y >= size) {
        y %= size;
      } else if (y < 0) {
        y = size + (y % size);
      }
    } else if (x >= size || y >= size || x < 0 || y < 0) {
      buffer[_x] = s_color_transparent;
      continue;
    }
    
    buffer[_x] = DecodeTilePixel8BPP(
      tile_base,
      vram[map_base + (y / 8) * block_width + (x / 8)],
      x % 8,
      y % 8
    );
  }
}