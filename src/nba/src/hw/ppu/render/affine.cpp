/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "hw/ppu/ppu.hpp"

namespace nba::core {

void PPU::RenderLayerAffine(int id) {
  auto const& bg = mmio.bgcnt[2 + id];
  
  u16* buffer = buffer_bg[2 + id];
  
  int size;
  int block_width;
  u32 map_base  = bg.map_block * 2048;
  u32 tile_base = bg.tile_block * 16384;
  
  switch (bg.size) {
    case 0: size = 128;  block_width = 16;  break;
    case 1: size = 256;  block_width = 32;  break;
    case 2: size = 512;  block_width = 64;  break;
    case 3: size = 1024; block_width = 128; break;
  }
  
  AffineRenderLoop(id, size, size, [&](int line_x, int x, int y) {
    auto tile_number = vram[map_base + (y / 8) * block_width + (x / 8)];
    buffer[line_x] = DecodeTilePixel8BPP(
      tile_base + tile_number * 64,
      x % 8,
      y % 8
    );
  });
}

} // namespace nba::core
