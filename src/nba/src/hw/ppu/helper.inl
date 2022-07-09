/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

auto ReadPalette(int palette, int index) -> u16 {
  return read<u16>(pram_draw, palette * 32 + index * 2) & 0x7FFF;
}

void DecodeTileLine4BPP(u16* buffer, u32 base, int palette, int number, int y, bool flip) {
  int xor_x = flip ? 7 : 0;
  u32 data = read<u32>(vram_draw, base + number * 32 + y * 4);

  for (int x = 0; x < 8; x++) {
    int index = data & 15;

    buffer[x ^ xor_x] = index ? ReadPalette(palette, index) : s_color_transparent;
    data >>= 4;
  }
}

void DecodeTileLine8BPP(u16* buffer, u32 base, int number, int y, bool flip) {
  int xor_x = flip ? 7 : 0;
  u64 data = read<u64>(vram_draw, base + number * 64 + y * 8);

  for (int x = 0; x < 8; x++) {
    int index = data & 0xFF;

    buffer[x ^ xor_x] = index ? ReadPalette(0, index) : s_color_transparent;
    data >>= 8;
  }
}

auto DecodeTilePixel4BPP(u32 address, int palette, int x, int y) -> u16 {
  u32 offset = address + (y * 4) + (x / 2);

  int tuple = vram_draw[offset];
  int index = (x & 1) ? (tuple >> 4) : (tuple & 0xF);

  if (index == 0) {
    return s_color_transparent;
  } else {
    return ReadPalette(palette, index);
  }
}

auto DecodeTilePixel8BPP(u32 address, int x, int y, bool sprite = false) -> u16 {
  u32 offset = address + (y * 8) + x;

  int index = vram_draw[offset];

  if (index == 0) {
    return s_color_transparent;
  } else {
    return ReadPalette(sprite ? 16 : 0, index);
  }
}

void AffineRenderLoop(int id,
                      int width,
                      int height,
                      int vcount,
                      std::function<void(int, int, int)> render_func) {
  auto& mmio = mmio_copy[vcount];
  auto const& bg = mmio.bgcnt[2 + id];
  auto const& mosaic = mmio.mosaic.bg;
  u16* buffer = buffer_bg[2 + id];
  
  s32 ref_x = mmio.bgx[id]._current;
  s32 ref_y = mmio.bgy[id]._current;
  s16 pa = mmio.bgpa[id];
  s16 pc = mmio.bgpc[id];
  
  int mosaic_x = 0;
  
  for (int _x = 0; _x < 240; _x++) {
    s32 x = ref_x >> 8;
    s32 y = ref_y >> 8;
    
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
      if (x >= width) {
        x %= width;
      } else if (x < 0) {
        x = width + (x % width);
      }
      
      if (y >= height) {
        y %= height;
      } else if (y < 0) {
        y = height + (y % height);
      }
    } else if (x >= width || y >= height || x < 0 || y < 0) {
      buffer[_x] = s_color_transparent;
      continue;
    }
    
    render_func(_x, (int)x, (int)y);
  }
}
