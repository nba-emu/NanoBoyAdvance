/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "ppu.hpp"

namespace nba::core {

static const int s_obj_size[4][4][2] = {
  // SQUARE
  {
    { 8 , 8  },
    { 16, 16 },
    { 32, 32 },
    { 64, 64 }
  },
  // HORIZONTAL
  {
    { 16, 8  },
    { 32, 8  },
    { 32, 16 },
    { 64, 32 }
  },
  // VERTICAL
  {
    { 8 , 16 },
    { 8 , 32 },
    { 16, 32 },
    { 32, 64 }
  },
  // PROHIBITED
  {
    { 0, 0 },
    { 0, 0 },
    { 0, 0 },
    { 0, 0 }
  }
};

void PPU::InitSprite() {
  const uint vcount = mmio.vcount;
  const u64 timestamp_now = scheduler.GetTimestampNow();

  sprite.timestamp_init = timestamp_now;
  sprite.timestamp_last_sync = timestamp_now;
  sprite.cycle = 0U;
  sprite.vcount = vcount;

  sprite.oam_fetch.index = 0U;
  sprite.oam_fetch.step = 0;
  sprite.oam_fetch.wait = 0;
  sprite.oam_fetch.delay_wait = false;
  sprite.drawing = false;
  sprite.state_rd = 0;
  sprite.state_wr = 1;

  sprite.buffer_rd = sprite.buffer[ vcount & 1];
  sprite.buffer_wr = sprite.buffer[(vcount & 1) ^ 1];

  std::memset(sprite.buffer_wr, 0, sizeof(Sprite::Pixel) * 240);
}

void PPU::DrawSprite() {
  const u64 timestamp_now = scheduler.GetTimestampNow();

  const int cycles = (int)(timestamp_now - sprite.timestamp_last_sync);

  if(cycles == 0 || sprite.cycle >= 1232U) {
    return;
  }

  // fmt::print("PPU: draw sprites @ VCOUNT={} cycles={}\n", mmio.vcount, cycles);

  DrawSpriteImpl(cycles);

  sprite.timestamp_last_sync = timestamp_now;
}

void PPU::DrawSpriteImpl(int cycles) {
  // @todo: in unlocked H-blank mode VRAM fetch appears to stop at cycle 960?
  const uint cycle_limit = mmio.dispcnt.hblank_oam_access ? 964U : 1232U;

  for(int i = 0; i < cycles; i++) {
    const uint cycle = sprite.cycle;

    // @todo: research how real HW handles the OBJ layer enable bit
    if(mmio.dispcnt.enable[LAYER_OBJ] && (cycle & 1U) == 0U) {
      DrawSpriteFetchVRAM(cycle);
      DrawSpriteFetchOAM(cycle);
    }

    if(++sprite.cycle == cycle_limit) {
      break;
    }
  }
}

void PPU::DrawSpriteFetchOAM(uint cycle) {
  auto& oam_fetch = sprite.oam_fetch;

  if(oam_fetch.wait > 0 && !oam_fetch.delay_wait) {
    oam_fetch.wait--;
    return;
  }

  oam_fetch.delay_wait = false;

  auto& drawer_state = sprite.drawer_state[sprite.state_wr];

  const auto Submit = [&](int wait) {
    // Swap the draw states and engage the drawer unit.
    sprite.state_rd ^= 1;
    sprite.state_wr ^= 1;
    sprite.drawing = true;

    // Start fetching the next OAM entry
    oam_fetch.index++;
    oam_fetch.step = 0;
    oam_fetch.wait = wait;
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
          if(y >= 160) y -= 256;

          const uint shape = (attr01 >> 14) & 3U;
          const uint size  =  attr01 >> 30;

          const int width  = s_obj_size[shape][size][0];
          const int height = s_obj_size[shape][size][1];

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

          // @todo: this can be precalculated at the start of the scanline
          // @todo: implement vertical sprite mosaic
          const int line = (sprite.vcount + 1) % 228;

          const int center_x = x + half_width;
          const int center_y = y + half_height;
          const int local_y = line - center_y;

          if(local_y >= -half_height && local_y < half_height) {
            active = true;

            // @todo: decode the mosaic bit
            drawer_state.x = center_x;
            drawer_state.y = center_y;
            drawer_state.width = width;
            drawer_state.height = height;
            drawer_state.half_width = half_width;
            drawer_state.half_height = half_height;
            drawer_state.mode = mode;
            drawer_state.affine = affine;
            drawer_state.local_x = -half_width;
            drawer_state.local_y = local_y;

            drawer_state.is_256 = (attr01 >> 13) & 1;

            oam_fetch.address = (((attr01 >> 25) & 31U) * 32U) + 6U;

            if(!affine) {
              const bool flip_v = attr01 & (1 << 29);

              drawer_state.flip_h = attr01 & (1 << 28);

              drawer_state.texture_x = 0;
              drawer_state.texture_y = line - y;

              if(flip_v) {
                drawer_state.texture_y ^= height - 1;
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
        Submit(drawer_state.half_width - 2);
      }
      break;
    }
    case 2:
    case 3:
    case 4:
    case 5: { // Fetch matrix components PA - PD (affine sprites only)
      drawer_state.matrix[step - 2] = FetchOAM<s16>(cycle, oam_fetch.address);
      
      oam_fetch.address += 8U;

      if(++oam_fetch.step == 6) {
        const int x0 = -drawer_state.half_width;
        const int y0 =  drawer_state.local_y;

        // @todo: document why the '<< 7' shift is required
        drawer_state.texture_x = (drawer_state.matrix[0] * x0 + drawer_state.matrix[1] * y0) + (drawer_state.width  << 7);
        drawer_state.texture_y = (drawer_state.matrix[2] * x0 + drawer_state.matrix[3] * y0) + (drawer_state.height << 7);

        Submit(drawer_state.half_width * 2 - 1);
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

    if(drawer_state.mode == OBJ_WINDOW) {
      if(opaque) pixel.window = 1;
    } else if(drawer_state.priority < pixel.priority || pixel.color == 0U) {
      if(opaque) {
        pixel.color = color;
        pixel.alpha = (drawer_state.mode == OBJ_SEMI) ? 1U : 0U;
      }

      pixel.mosaic = 0; // @todo
      pixel.priority = drawer_state.priority;
    }
  };

  if(drawer_state.affine) {
    if(sprite.oam_fetch.delay_wait) {
      return;
    }

    const int x = drawer_state.x + drawer_state.local_x;
    
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

      Plot(x, color_index);
    }
  
    drawer_state.texture_x += drawer_state.matrix[0];
    drawer_state.texture_y += drawer_state.matrix[2];

    if(++drawer_state.local_x == drawer_state.half_width) {
      sprite.drawing = false;
    }
  } else {
    const bool flip_h = drawer_state.flip_h;

    const int texture_x = drawer_state.texture_x ^ (flip_h ? (width - 1) : 0);
    const int texture_y = drawer_state.texture_y;

    const int tile_x  = texture_x & 7;
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

    // @todo: try to optimize this a bit.
    for(int i = 0; i < 2; i++) {
      const int x = drawer_state.x + drawer_state.local_x;

      uint color_index = color_indices[i];

      if(color_index > 0U) {
        color_index |= palette;
      }

      Plot(x, color_index);

      if(++drawer_state.local_x == drawer_state.half_width) {
        sprite.drawing = false;
      }
    }

    drawer_state.texture_x += 2;
  }
}

} // namespace nba::core
