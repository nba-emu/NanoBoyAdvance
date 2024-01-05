
#pragma once

#include <nba/integer.hpp>

constexpr u32 Rgb565ToArgb8888(u16 color_rgb565) {
  const uint r =  (color_rgb565 >>  0) & 31;
  const uint g = ((color_rgb565 >>  4) & 62) | (color_rgb565 >> 15);
  const uint b =  (color_rgb565 >> 10) & 31;

  return 0xFF000000u |
         (r << 3 | r >> 2) << 16 |
         (g << 2 | g >> 4) <<  8 |
         (b << 3 | b >> 2);
}