/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "../ppu.hpp"

using namespace GameBoyAdvance;

void PPU::RenderLayerAffine(int id) {
  auto const& bg = mmio.bgcnt[2 + id];
  
  std::uint16_t* buffer = buffer_bg[2 + id];
  
  int size;
  int block_width;
  std::uint32_t map_base  = bg.map_block * 2048;
  std::uint32_t tile_base = bg.tile_block * 16384;
  
  switch (bg.size) {
    case 0: size = 128;  block_width = 16;  break;
    case 1: size = 256;  block_width = 32;  break;
    case 2: size = 512;  block_width = 64;  break;
    case 3: size = 1024; block_width = 128; break;
  }
  
  AffineRenderLoop(id, size, size, [&](int line_x, int x, int y) {
    buffer[line_x] = DecodeTilePixel8BPP(
      tile_base,
      vram[map_base + (y / 8) * block_width + (x / 8)],
      x % 8,
      y % 8
    );
  });
}