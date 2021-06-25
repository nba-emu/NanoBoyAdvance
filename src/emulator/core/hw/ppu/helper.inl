/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

auto ReadPalette(int palette, int index) -> u16 {
  int cell = (palette * 32) + (index * 2);

  /* TODO: On Little-Endian devices we can get away with casting to uint16_t*. */
  return ((pram[cell + 1] << 8) |
           pram[cell + 0]) & 0x7FFF;
}

void DecodeTileLine4BPP(u16* buffer, u32 base, int palette, int number, int y, bool flip) {
  u8* data = &vram[base + (number * 32) + (y * 4)];

  if (flip) {
    for (int x = 0; x < 4; x++) {
      int d  = *data++;
      int p1 = d & 15;
      int p2 = d >> 4;

      buffer[(x*2+0)^7] = p1 ? ReadPalette(palette, p1) : s_color_transparent;
      buffer[(x*2+1)^7] = p2 ? ReadPalette(palette, p2) : s_color_transparent;
    }
  } else {
    for (int x = 0; x < 4; x++) {
      int d  = *data++;
      int p1 = d & 15;
      int p2 = d >> 4;

      buffer[x*2+0] = p1 ? ReadPalette(palette, p1) : s_color_transparent;
      buffer[x*2+1] = p2 ? ReadPalette(palette, p2) : s_color_transparent;
    }
  }
}

void DecodeTileLine8BPP(u16* buffer, u32 base, int number, int y, bool flip) {
  u8* data = &vram[base + (number * 64) + (y * 8)];

  if (flip) {
    for (int x = 7; x >= 0; x--) {
      int pixel = *data++;
      buffer[x] = pixel ? ReadPalette(0, pixel) : s_color_transparent;
    }
  } else {
    for (int x = 0; x < 8; x++) {
      int pixel = *data++;
      buffer[x] = pixel ? ReadPalette(0, pixel) : s_color_transparent;
    }
  }
}

auto DecodeTilePixel4BPP(u32 address, int palette, int x, int y) -> u16 {
  u32 offset = address + (y * 4) + (x / 2);

  int tuple = vram[offset];
  int index = (x & 1) ? (tuple >> 4) : (tuple & 0xF);

  if (index == 0) {
    return s_color_transparent;
  } else {
    return ReadPalette(palette, index);
  }
}

auto DecodeTilePixel8BPP(u32 address, int x, int y, bool sprite = false) -> u16 {
  u32 offset = address + (y * 8) + x;

  int index = vram[offset];

  if (index == 0) {
    return s_color_transparent;
  } else {
    return ReadPalette(sprite ? 16 : 0, index);
  }
}

void AffineRenderLoop(int id,
                      int width,
                      int height,
                      std::function<void(int, int, int)> render_func) {
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
