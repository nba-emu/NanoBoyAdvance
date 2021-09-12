/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "hw/ppu/ppu.hpp"

namespace nba::core {

void PPU::RenderLayerBitmap1() {
  AffineRenderLoop(0, 240, 160, [&](int line_x, int x, int y) {
    int index = y * 480 + x * 2;
    
    buffer_bg[2][line_x] = (vram[index + 1] << 8) | vram[index];
  });
}

void PPU::RenderLayerBitmap2() {  
  auto frame = mmio.dispcnt.frame * 0xA000;
  
  AffineRenderLoop(0, 240, 160, [&](int line_x, int x, int y) {
    int index = vram[frame + y * 240 + x];
    
    if (index == 0) {
      buffer_bg[2][line_x] = s_color_transparent;
    } else {
      buffer_bg[2][line_x] = ReadPalette(0, index);
    }
  });
}

void PPU::RenderLayerBitmap3() {
  auto frame = mmio.dispcnt.frame * 0xA000;
  
  AffineRenderLoop(0, 160, 128, [&](int line_x, int x, int y) {
    int index = frame + y * 320 + x * 2;
    
    buffer_bg[2][line_x] = (vram[index + 1] << 8) | vram[index];
  });
}

} // namespace nba::core
