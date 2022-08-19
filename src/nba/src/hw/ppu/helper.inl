/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

auto ReadPalette(int palette, int index) -> u16 {
  return read<u16>(pram, palette * 32 + index * 2) & 0x7FFF;
}

auto DecodeTilePixel4BPP(u32 address, int palette, int x, int y) -> u32 {
  u32 offset = address + (y * 4) + (x / 2);

  int tuple = vram[offset];
  int index = (x & 1) ? (tuple >> 4) : (tuple & 0xF);

  if (index == 0) {
    return 0x8000'0000;
  } else {
    return ReadPalette(palette, index) | 0x4000'0000;
  }
}

auto DecodeTilePixel8BPP(u32 address, int x, int y, bool sprite = false) -> u32 {
  u32 offset = address + (y * 8) + x;

  int index = vram[offset];

  if (index == 0) {
    return 0x8000'0000;
  } else {
    return ReadPalette(sprite ? 16 : 0, index) | 0x4000'0000;
  }
}

