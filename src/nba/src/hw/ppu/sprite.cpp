/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <algorithm>

#include "ppu.hpp"

namespace nba::core {

void PPU::InitSprite() {
  const uint vcount = mmio.vcount;
  const u64 timestamp_now = scheduler.GetTimestampNow();

  sprite.timestamp_init = timestamp_now;
  sprite.timestamp_last_sync = timestamp_now;
  sprite.cycle = 0U;
  sprite.vcount = (vcount + 1) % 228;
  sprite.mosaic_y = mmio.mosaic.obj._counter_y;

  sprite.oam_fetch.index = 0U;
  sprite.oam_fetch.step = 0;
  sprite.oam_fetch.wait = 0;
  sprite.oam_fetch.delay_wait = false;
  sprite.drawing = false;
  sprite.state_rd = 0;
  sprite.state_wr = 1;

  // @todo: figure how the cycle limit is implemented in HW.
  // @todo: in unlocked H-blank mode VRAM fetch appears to stop at cycle 960?
  sprite.latch_cycle_limit = mmio.dispcnt.hblank_oam_access ? 964U : 1232U;

  std::memset(sprite.buffer_wr, 0, sizeof(Sprite::Pixel) * 240);
}

void PPU::DrawSprite() {
  const u64 timestamp_now = scheduler.GetTimestampNow();

  const int cycles = (int)(timestamp_now - sprite.timestamp_last_sync);

  if(cycles == 0 || sprite.cycle >= sprite.latch_cycle_limit) {
    return;
  }

  DrawSpriteImpl(cycles);

  sprite.timestamp_last_sync = timestamp_now;
}

void PPU::DrawSpriteImpl(int cycles) {
  const uint cycle_limit = sprite.latch_cycle_limit;

  for(int i = 0; i < cycles; i++) {
    const uint cycle = sprite.cycle;

    // @todo: research how real HW handles the OBJ layer enable bit
    if(mmio.dispcnt.enable[LAYER_OBJ] && (cycle & 1U) == 0U) {
      DrawSpriteFetchVRAM(cycle);
      DrawSpriteFetchOAM(cycle);
    }

    if(cycle == 1192U) { // cycle 1232 in the scanline
      auto& mosaic = mmio.mosaic;

      if(sprite.vcount < 159) {
        if(++mosaic.obj._counter_y == mosaic.obj.size_y) {
          mosaic.obj._counter_y = 0;
        } else {
          mosaic.obj._counter_y &= 15;
        }
      } else {
        mosaic.obj._counter_y = 0;
      }
    }

    if(++sprite.cycle == cycle_limit) {
      break;
    }
  }
}

void PPU::DrawSpriteFetchOAM(uint cycle) {
  static constexpr int k_sprite_size[4][4][2] = {
    { { 8 , 8  }, { 16, 16 }, { 32, 32 }, { 64, 64 } }, // Square
    { { 16, 8  }, { 32, 8  }, { 32, 16 }, { 64, 32 } }, // Horizontal
    { { 8 , 16 }, { 8 , 32 }, { 16, 32 }, { 32, 64 } }, // Vertical
    { { 8 , 8  }, { 8 , 8  }, { 8 , 8  }, { 8 , 8  } }  // Prohibited
  };

  auto& oam_fetch = sprite.oam_fetch;

  if(oam_fetch.wait > 0 && !oam_fetch.delay_wait) {
    oam_fetch.wait--;
    return;
  }

  oam_fetch.delay_wait = false;

  auto& drawer_state = sprite.drawer_state[sprite.state_wr];

  const auto Submit = [&]() {
    // Swap the draw states and engage the drawer unit.
    sprite.state_rd ^= 1;
    sprite.state_wr ^= 1;
    sprite.drawing = true;

    // Start fetching the next OAM entry
    oam_fetch.index++;
    oam_fetch.step = 0;
    oam_fetch.wait = oam_fetch.pending_wait;
    oam_fetch.delay_wait = true;
  };

  const int step = oam_fetch.step;

  switch(step) {
    case 0: { // Fetch Attribute #0 and #1 
      if(oam_fetch.index == 128U) {
        oam_fetch.step = 6; // we are done!
        break;
      }

      const u32 attr01 = FetchOAM<u32>(cycle, oam_fetch.index * 8U);

      bool active = false;

      if((attr01 & 0x300U) != 0x200U) { // check if the sprite is enabled
        const uint mode = (attr01 >> 10) & 3U;

        // @todo: how does HW handle OBJs in prohibited mode?
        if(mode != OBJ_PROHIBITED) {
          s32 x = (attr01 >> 16) & 0x1FF;
          s32 y =  attr01 & 0xFF;

          if(x >= 240) x -= 512;

          const uint shape = (attr01 >> 14) & 3U;
          const uint size  =  attr01 >> 30;

          const int width  = k_sprite_size[shape][size][0];
          const int height = k_sprite_size[shape][size][1];

          int half_width  = width  >> 1;
          int half_height = height >> 1;

          const bool affine = attr01 & 0x100U;

          if(affine) {
            const bool double_size = attr01 & 0x200U;

            if(double_size) {
              half_width  *= 2;
              half_height *= 2;
            }
          }

          const int vcount = sprite.vcount;

          const int y_max = (y + half_height * 2) & 255;

          if((vcount >= y || y_max < y) && vcount < y_max) {
            const bool mosaic = attr01 & (1 << 12);

            const int line = sprite.vcount - (mosaic ? sprite.mosaic_y : 0);

            drawer_state.width = width;
            drawer_state.height = height;
            drawer_state.mode = mode;
            drawer_state.mosaic = mosaic;
            drawer_state.affine = affine;
            drawer_state.draw_x = x;
            drawer_state.remaining_pixels = half_width << 1;
       
            drawer_state.is_256 = (attr01 >> 13) & 1;

            if(!affine) {
              const bool flip_v = attr01 & (1 << 29);

              drawer_state.flip_h = attr01 & (1 << 28);

              drawer_state.texture_x = 0;
              drawer_state.texture_y = (line - y) & 255;

              if(flip_v) {
                drawer_state.texture_y ^= height - 1;
              }

              oam_fetch.pending_wait = half_width - 2;
            } else {
              oam_fetch.initial_local_x = -half_width;
              oam_fetch.initial_local_y = line - (y_max - half_height);
              oam_fetch.pending_wait = half_width * 2 - 1;
              oam_fetch.matrix_address = (((attr01 >> 25) & 31U) * 32U) + 6U;
            }

            active = true;

            if(x < 0) {
              const int clip = -x & (affine ? ~0 : ~1);

              drawer_state.draw_x += clip;
              drawer_state.remaining_pixels -= clip;

              if(affine) {
                oam_fetch.pending_wait -= clip;
                oam_fetch.initial_local_x += clip;
              } else {
                oam_fetch.pending_wait -= clip >> 1;
                drawer_state.texture_x += clip;
              }

              if(drawer_state.remaining_pixels <= 0) {
                active = false;
              }
            }
          }
        }
      }

      if(active) {
        oam_fetch.step = 1;
      } else {
        oam_fetch.index++;
      }
      break;
    }
    case 1: { // Fetch Attribute #2
      const u16 attr2 = FetchOAM<u16>(cycle, oam_fetch.index * 8U + 4U);

      drawer_state.tile_number = attr2 & 0x3FFU;
      drawer_state.priority = (attr2 >> 10) & 3U;
      drawer_state.palette = attr2 >> 12;

      if(drawer_state.affine) {
        oam_fetch.step = 2;
      } else {
        Submit();
      }
      break;
    }
    case 2:
    case 3:
    case 4:
    case 5: { // Fetch matrix components PA - PD (affine sprites only)
      drawer_state.matrix[step - 2] = FetchOAM<s16>(cycle, oam_fetch.matrix_address);
      
      oam_fetch.matrix_address += 8U;

      if(++oam_fetch.step == 6) {
        const int x0 = oam_fetch.initial_local_x;
        const int y0 = oam_fetch.initial_local_y;

        // @todo: document why the '<< 7' shift is required
        drawer_state.texture_x = (drawer_state.matrix[0] * x0 + drawer_state.matrix[1] * y0) + (drawer_state.width  << 7);
        drawer_state.texture_y = (drawer_state.matrix[2] * x0 + drawer_state.matrix[3] * y0) + (drawer_state.height << 7);

        Submit();
      }
      break;
    }
  }
}

void PPU::DrawSpriteFetchVRAM(uint cycle) {
  if(!sprite.drawing) {
    return;
  }

  auto& drawer_state = sprite.drawer_state[sprite.state_rd];

  const int width  = drawer_state.width;
  const int height = drawer_state.height;

  const uint base_tile = drawer_state.tile_number;

  const auto CalculateTileNumber4BPP = [&](int block_x, int block_y) -> uint {
    // @todo: can the OAM mapping be changed mid-scanline? what would that do?
    if(mmio.dispcnt.oam_mapping_1d) {
      return (base_tile + block_y * ((uint)width >> 3) + block_x) & 0x3FFU;
    }

    return ((base_tile + (block_y << 5)) & 0x3E0U) | ((base_tile + block_x) & 0x1FU);
  };

  const auto CalculateTileNumber8BPP = [&](int block_x, int block_y) -> uint {
    // @todo: can the OAM mapping be changed mid-scanline? what would that do?
    if(mmio.dispcnt.oam_mapping_1d) {
      return (base_tile + block_y * ((uint)width >> 2) + (block_x << 1)) & 0x3FFU;
    }

    return ((base_tile + (block_y << 5)) & 0x3E0U) | (((base_tile & ~1) + (block_x << 1)) & 0x1FU);
  };

  const auto CalculateAddress4BPP = [&](uint tile, int tile_x, int tile_y) -> uint {
    return 0x10000U + (tile << 5) + (tile_y << 2) + (tile_x >> 1);
  };

  const auto CalculateAddress8BPP = [&](uint tile, int tile_x, int tile_y) -> uint {
    return 0x10000U + (tile << 5) + (tile_y << 3) + tile_x;
  };

  const auto Plot = [&](int x, uint color) {
    if(x < 0 || x >= 240) return;

    auto& pixel = sprite.buffer_wr[x];

    const bool opaque = color != 0U;
    const auto mode = drawer_state.mode;
    const uint priority = drawer_state.priority;

    /**
     * Transparent/outside OBJ window pixels are treated the same as
     * normal/semi-transparent sprite pixels, meaning that (unlike opaque/inside OBJ window pixels) they
     * update the mosaic and priority attributes.
     */
    if(mode == OBJ_WINDOW && opaque) {
      pixel.window = 1;
    } else if(priority < pixel.priority || pixel.color == 0U) {
      if(opaque) {
        pixel.color = color;
        pixel.alpha = (mode == OBJ_SEMI) ? 1U : 0U;
      }
      pixel.mosaic = drawer_state.mosaic ? 1U : 0U;
      pixel.priority = priority;
    }
  };

  if(drawer_state.affine) {
    if(sprite.oam_fetch.delay_wait) {
      return;
    }

    const int texture_x = drawer_state.texture_x >> 8;
    const int texture_y = drawer_state.texture_y >> 8;

    if(texture_x >= 0 && texture_x < width &&
       texture_y >= 0 && texture_y < height) {

      const int tile_x  = texture_x & 7;
      const int tile_y  = texture_y & 7;
      const int block_x = texture_x >> 3;
      const int block_y = texture_y >> 3;

      uint color_index = 0U;

      if(drawer_state.is_256) {
        const uint tile = CalculateTileNumber8BPP(block_x, block_y);

        color_index = FetchVRAM_OBJ<u8>(cycle, CalculateAddress8BPP(tile, tile_x, tile_y));
      } else {
        const uint tile = CalculateTileNumber4BPP(block_x, block_y);
        const u8 data = FetchVRAM_OBJ<u8>(cycle, CalculateAddress4BPP(tile, tile_x, tile_y));

        if(tile_x & 1U) {
          color_index = data >> 4;
        } else {
          color_index = data & 15U;
        }

        if(color_index > 0U) {
          color_index |= drawer_state.palette << 4;
        }
      }

      Plot(drawer_state.draw_x, color_index);
    }
  
    drawer_state.draw_x++;

    drawer_state.texture_x += drawer_state.matrix[0];
    drawer_state.texture_y += drawer_state.matrix[2];

    if(--drawer_state.remaining_pixels == 0) sprite.drawing = false;
  } else {
    const bool flip_h = drawer_state.flip_h;

    const int texture_x = drawer_state.texture_x ^ (flip_h ? (width - 1) : 0);
    const int texture_y = drawer_state.texture_y;

    const int tile_x  = texture_x & 7 & ~1;
    const int tile_y  = texture_y & 7;
    const int block_x = texture_x >> 3;
    const int block_y = texture_y >> 3;

    uint palette;
    uint color_indices[2] {0, 0};

    if(drawer_state.is_256) {
      const uint tile = CalculateTileNumber8BPP(block_x, block_y);
      const u16 data = FetchVRAM_OBJ<u16>(cycle, CalculateAddress8BPP(tile, tile_x, tile_y));

      if(flip_h) {
        color_indices[0] = data >> 8;
        color_indices[1] = data & 0xFFU;
      } else {
        color_indices[0] = data & 0xFFU;
        color_indices[1] = data >> 8;
      }

      palette = 0U;
    } else {
      const uint tile = CalculateTileNumber4BPP(block_x, block_y);
      const u8 data = FetchVRAM_OBJ<u8>(cycle, CalculateAddress4BPP(tile, tile_x, tile_y));

      if(flip_h) {
        color_indices[0] = data >> 4;
        color_indices[1] = data & 15U;
      } else {
        color_indices[0] = data & 15U;
        color_indices[1] = data >> 4;
      }

      palette = drawer_state.palette << 4;
    }

    for(int i = 0; i < 2; i++) {
      uint color_index = color_indices[i];

      if(color_index > 0U) {
        color_index |= palette;
      }

      Plot(drawer_state.draw_x++, color_index);
    }

    drawer_state.texture_x += 2;
    
    drawer_state.remaining_pixels -= 2;

    if(drawer_state.remaining_pixels == 0) sprite.drawing = false;
  }
}

} // namespace nba::core
