/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "../ppu.hpp"

namespace nba::core {

const int PPU::s_obj_size[4][4][2] = {
  /* SQUARE */
  {
    { 8 , 8  },
    { 16, 16 },
    { 32, 32 },
    { 64, 64 }
  },
  /* HORIZONTAL */
  {
    { 16, 8  },
    { 32, 8  },
    { 32, 16 },
    { 64, 32 }
  },
  /* VERTICAL */
  {
    { 8 , 16 },
    { 8 , 32 },
    { 16, 32 },
    { 32, 64 }
  },
  /* PROHIBITED */
  {
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 }
  }
};

void PPU::RenderLayerOAM(bool bitmap_mode, int line) {
  int tile_num;
  std::uint16_t pixel;
  std::int16_t transform[4];
  int cycles = mmio.dispcnt.hblank_oam_access ? 954 : 1210;

  line_contains_alpha_obj = false;

  for (int x = 0; x < 240; x++) {
    buffer_obj[x].priority = 4;
    buffer_obj[x].color = s_color_transparent;
    buffer_obj[x].alpha = 0;
    buffer_obj[x].window = 0;
  }

  for (std::int32_t offset = 0; offset <= 127 * 8; offset += 8) {
    if ((oam[offset + 1] & 3) == 2) {
      continue;
    }

    std::uint16_t attr0 = (oam[offset + 1] << 8) | oam[offset + 0];
    std::uint16_t attr1 = (oam[offset + 3] << 8) | oam[offset + 2];
    std::uint16_t attr2 = (oam[offset + 5] << 8) | oam[offset + 4];

    int width;
    int height;

    std::int32_t x = attr1 & 0x1FF;
    std::int32_t y = attr0 & 0x0FF;
    int shape  =  attr0 >> 14;
    int size   =  attr1 >> 14;
    int prio   = (attr2 >> 10) & 3;
    int mode   = (attr0 >> 10) & 3;
    int mosaic = (attr0 >> 12) & 1;

    if (mode == OBJ_PROHIBITED) {
      continue;
    }

    if (x >= 240) x -= 512;
    if (y >= 160) y -= 256;

    int affine  = (attr0 >> 8) & 1;
    int attr0b9 = (attr0 >> 9) & 1;

    width  = s_obj_size[shape][size][0];
    height = s_obj_size[shape][size][1];

    int half_width  = width / 2;
    int half_height = height / 2;

    if (affine) {
      int group = ((attr1 >> 9) & 0x1F) << 5;

      transform[0] = (oam[group + 0x7 ] << 8) | oam[group + 0x6 ];
      transform[1] = (oam[group + 0xF ] << 8) | oam[group + 0xE ];
      transform[2] = (oam[group + 0x17] << 8) | oam[group + 0x16];
      transform[3] = (oam[group + 0x1F] << 8) | oam[group + 0x1E];

      if (attr0b9) {
        half_width  *= 2;
        half_height *= 2;
      }
    } else {
      transform[0] = 0x100;
      transform[1] = 0;
      transform[2] = 0;
      transform[3] = 0x100;
    }

    x += half_width;
    y += half_height;

    if (line < (y - half_height) || line >= (y + half_height)) {
      continue;
    }

    int local_y = line - y;
    int number  =  attr2 & 0x3FF;
    int palette = (attr2 >> 12) + 16;
    int flip_h  = !affine && (attr1 & (1 << 12));
    int flip_v  = !affine && (attr1 & (1 << 13));
    int is_256  = (attr0 >> 13) & 1;

    std::uint32_t tile_base = 0x10000;

    int mosaic_x = 0;

    if (mosaic) {
      mosaic_x = (x - half_width) % mmio.mosaic.obj.size_x;
      local_y -= mmio.mosaic.obj._counter_y;
    }

    for (int local_x = -half_width; local_x < half_width; local_x++) {
      int _local_x = local_x - mosaic_x;
      int global_x = local_x + x;

      if (mosaic && (++mosaic_x == mmio.mosaic.obj.size_x)) {
        mosaic_x = 0;
      }

      if (global_x < 0 || global_x >= 240) {
        continue;
      }

      int tex_x = ((transform[0] * _local_x + transform[1] * local_y) >> 8) + (width / 2);
      int tex_y = ((transform[2] * _local_x + transform[3] * local_y) >> 8) + (height / 2);

      if (tex_x >= width || tex_y >= height ||
        tex_x < 0 || tex_y < 0) {
        continue;
      }

      if (flip_h) tex_x = width  - tex_x - 1;
      if (flip_v) tex_y = height - tex_y - 1;

      int tile_x  = tex_x % 8;
      int tile_y  = tex_y % 8;
      int block_x = tex_x / 8;
      int block_y = tex_y / 8;

      if (is_256) {
        if (mmio.dispcnt.oam_mapping_1d) {
          tile_num = number + block_y * (width / 4);
        } else {
          tile_num = (number & ~1) + block_y * 32;
        }

        tile_num += block_x * 2;

        if (bitmap_mode && tile_num < 512) {
          continue;
        }

        pixel = DecodeTilePixel8BPP(tile_base + tile_num * 32, tile_x, tile_y, true);
      } else {
        if (mmio.dispcnt.oam_mapping_1d) {
          tile_num = number + block_y * (width / 8);
        } else {
          tile_num = number + block_y * 32;
        }

        tile_num += block_x;

        if (bitmap_mode && tile_num < 512) {
          continue;
        }

        pixel = DecodeTilePixel4BPP(tile_base + tile_num * 32, palette, tile_x, tile_y);
      }

      auto& point = buffer_obj[global_x];
      bool opaque = pixel != s_color_transparent;

      if (mode == OBJ_WINDOW) {
        if (opaque) point.window = 1;
      } else if (prio < point.priority || point.color == s_color_transparent) {
        if (opaque) {
          point.color = pixel;
          point.alpha = (mode == OBJ_SEMI) ? 1 : 0;
          line_contains_alpha_obj |= point.alpha;
        }

        point.priority = prio;
      }
    }

    if (affine) {
      cycles -= 10 + half_width * 4;
    } else {
      cycles -= half_width * 2;
    }

    if (cycles <= 0) {
      break;
    }
  }
}

} // namespace nba::core
