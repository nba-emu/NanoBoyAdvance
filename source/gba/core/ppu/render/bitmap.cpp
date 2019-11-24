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
  AffineRenderLoop(0, 240, 160, [&](int line_x, int x, int y) {
    int index = y * 480 + x * 2;
    
    buffer_bg[2][line_x] = (vram[index + 1] << 8) | vram[index];
  });
}

void PPU::RenderLayerBitmap2() {  
  auto frame = mmio.dispcnt.frame * 0xA000;
  
  AffineRenderLoop(0, 240, 160, [&](int line_x, int x, int y) {
    int index = frame + y * 240 + x;
    
    buffer_bg[2][line_x] = ReadPalette(0, vram[index]);
  });
}

void PPU::RenderLayerBitmap3() {
  auto frame = mmio.dispcnt.frame * 0xA000;
  
  AffineRenderLoop(0, 160, 128, [&](int line_x, int x, int y) {
    int index = frame + y * 320 + x * 2;
    
    buffer_bg[2][line_x] = (vram[index + 1] << 8) | vram[index];
  });
}
