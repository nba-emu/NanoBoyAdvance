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

void PPU::RenderLayerOAM() {  
  /* 2d-affine transform matrix (pa, pb, pc, pd). */
  std::int16_t transform[4];

  int tile_num;
  std::uint16_t pixel;
  std::uint32_t tile_base = 0x10000;
  std::int32_t  offset = 127 * 8;
  
  line_contains_alpha_obj = false;
  
  /* TODO: check if there is a faster way to initialize the array. */
  for (int x = 0; x < 240; x++) {
    buffer_obj[x].priority = 4;
    buffer_obj[x].color = s_color_transparent;
    buffer_obj[x].alpha = 0;
    buffer_obj[x].window = 0;
  }

  for (; offset >= 0; offset -= 8) {
    /* Check if OBJ is diabled (affine=0, attr0bit9=1) */
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

    /* Decode OBJ width and height. */
    width  = s_obj_size[shape][size][0];
    height = s_obj_size[shape][size][1];

    int rect_width  = width;
    int rect_height = height;

    int half_width  = width / 2;
    int half_height = height / 2;

    /* Set x and y to OBJ origin. */
    x += half_width;
    y += half_height;

    /* Load transform matrix. */
    if (affine) {
      int group = ((attr1 >> 9) & 0x1F) << 5;

      /* Read transform matrix. */
      transform[0] = (oam[group + 0x7 ] << 8) | oam[group + 0x6 ];
      transform[1] = (oam[group + 0xF ] << 8) | oam[group + 0xE ];
      transform[2] = (oam[group + 0x17] << 8) | oam[group + 0x16];
      transform[3] = (oam[group + 0x1F] << 8) | oam[group + 0x1E];

      /* Check double-size flag. Doubles size of the view rectangle. */
      if (attr0b9) {
        x += half_width;
        y += half_height;
        rect_width  *= 2;
        rect_height *= 2;
        half_width  *= 2;
        half_height *= 2;
      }
    } else {
      /* Set transform to identity:
       * [ 1 0 ]
       * [ 0 1 ]
       */
      transform[0] = 0x100;
      transform[1] = 0;
      transform[2] = 0;
      transform[3] = 0x100;
    }
    
    int line = mmio.vcount;

    /* Bail out if scanline is outside OBJ's view rectangle. */
    if (line < (y - half_height) || line >= (y + half_height)) {
      continue;
    }

    int local_y = line - y;
    int number  =  attr2 & 0x3FF;
    int palette = (attr2 >> 12) + 16;
    int flip_h  = !affine && (attr1 & (1 << 12));
    int flip_v  = !affine && (attr1 & (1 << 13));
    int is_256  = (attr0 >> 13) & 1;

    if (is_256) number /= 2;

    int mosaic_x = 0;
    
    if (mosaic) {
      /* TODO: optimize this operation. */
      mosaic_x = (x - half_width) % mmio.mosaic.obj.size_x;
      local_y -= mmio.mosaic.obj._counter_y;
    }
    
    /* Render OBJ scanline. */
    for (int local_x = -half_width; local_x <= half_width; local_x++) {
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

      /* Check if transformed coordinates are inside bounds. */
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

      if (mmio.dispcnt.oam_mapping_1d) {
        tile_num = number + block_y * (width / 8);
      } else {
        tile_num = number + block_y * (is_256 ? 16 : 32); /* check me */
      }
      
      tile_num += block_x;

      if (is_256) {
        pixel = DecodeTilePixel8BPP(tile_base, tile_num, tile_x, tile_y, true);
      } else {
        pixel = DecodeTilePixel4BPP(tile_base, palette, tile_num, tile_x, tile_y);
      }

      auto& point = buffer_obj[global_x];
      
      if (pixel != s_color_transparent) {
        if (mode == OBJ_WINDOW) {
          point.window = 1;
        } else if (prio <= point.priority) {
          point.priority = prio;
          point.color = pixel;
          point.alpha = (mode == OBJ_SEMI) ? 1 : 0;
          if (point.alpha) {
            line_contains_alpha_obj = true;
          }
        }
      }
    }
  }
}