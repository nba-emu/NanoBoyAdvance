/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "hw/ppu/ppu.hpp"

namespace nba::core {

void PPU::RenderLayerBitmap1(int vcount) {
  AffineRenderLoop(0, 240, 160, vcount, [&](int line_x, int x, int y) {
    int index = y * 480 + x * 2;
    
    buffer_bg[2][line_x] = (vram_draw[index + 1] << 8) | vram_draw[index];
  });
}

void PPU::RenderLayerBitmap2(int vcount) {  
  auto frame = mmio_copy[vcount].dispcnt.frame * 0xA000;
  
  AffineRenderLoop(0, 240, 160, vcount, [&](int line_x, int x, int y) {
    int index = vram_draw[frame + y * 240 + x];
    
    if (index == 0) {
      buffer_bg[2][line_x] = s_color_transparent;
    } else {
      buffer_bg[2][line_x] = ReadPalette(0, index);
    }
  });
}

void PPU::RenderLayerBitmap3(int vcount) {
  auto frame = mmio_copy[vcount].dispcnt.frame * 0xA000;
  
  AffineRenderLoop(0, 160, 128, vcount, [&](int line_x, int x, int y) {
    int index = frame + y * 320 + x * 2;
    
    buffer_bg[2][line_x] = (vram_draw[index + 1] << 8) | vram_draw[index];
  });
}

} // namespace nba::core
