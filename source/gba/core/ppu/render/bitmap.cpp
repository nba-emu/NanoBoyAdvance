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

void PPU::RenderLayerBitmap1() {
  auto line = mmio.vcount;
  auto mosaic = mmio.bgcnt[2].mosaic_enable;
  
  if (mosaic) {
    line -= mmio.mosaic.bg._counter_y;
  }
  
  auto offset = line * 480;
  
  int mosaic_x = 0;
  
  for (int x = 0; x < 240; x++) {
    int index = offset + (x - mosaic_x) * 2;
    
    buffer_bg[2][x] = (vram[index + 1] << 8) | vram[index];
  
    if (mosaic && (++mosaic_x == mmio.mosaic.bg.size_x)) {
      mosaic_x = 0;
    }
  }
}

void PPU::RenderLayerBitmap2() {  
  auto line = mmio.vcount;
  auto mosaic = mmio.bgcnt[2].mosaic_enable;
  
  if (mosaic) {
    line -= mmio.mosaic.bg._counter_y;
  }
  
  auto frame = mmio.dispcnt.frame * 0xA000;
  auto offset = frame + line * 240;

  int mosaic_x = 0;
  
  for (int x = 0; x < 240; x++) {
    buffer_bg[2][x] = ReadPalette(0, vram[offset + x - mosaic_x]);
    
    if (mosaic && (++mosaic_x == mmio.mosaic.bg.size_x)) {
      mosaic_x = 0;
    }
  }
}

void PPU::RenderLayerBitmap3() {
  auto line = mmio.vcount;
  auto mosaic = mmio.bgcnt[2].mosaic_enable;
  
  if (mosaic) {
    line -= mmio.mosaic.bg._counter_y;
  }
  
  auto frame = mmio.dispcnt.frame * 0xA000;
  auto offset = frame + line * 320;

  int mosaic_x = 0;
  
  for (int x = 0; x < 240; x++) {
    int _x = x - mosaic_x;
    
    /* TODO: should be check the mosaic-adjusted line or VCOUNT? */
    if (_x < 160 && line < 128) {
      int index = offset + _x * 2;
      
      buffer_bg[2][x] = (vram[index + 1] << 8) | vram[index];
    } else {
      buffer_bg[2][x] = s_color_transparent;
    }
    
    if (mosaic && (++mosaic_x == mmio.mosaic.bg.size_x)) {
      mosaic_x = 0;
    }
  }
}