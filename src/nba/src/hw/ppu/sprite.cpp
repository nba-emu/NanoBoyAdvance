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

  std::memset(sprite.buffer_wr, 0, sizeof(u32) * 240);
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

          const int center_x = x + half_width;
          const int center_y = y + half_height;
          const int local_y = (sprite.vcount + 1) % 228 - center_y;

          if(local_y >= -half_height && local_y < half_height) {
            active = true;

            // @todo: sprite mosaic
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

            drawer_state.flip_h = !affine && (attr01 & (1 << 28));
            drawer_state.flip_v = !affine && (attr01 & (1 << 29));
            drawer_state.is_256 = (attr01 >> 13) & 1;

            oam_fetch.address = (((attr01 >> 25) & 31U) * 32U) + 6U;
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

      drawer_state.tile_number = attr2 & 0x3FF;
      drawer_state.priority = (attr2 >> 10) & 3;
      drawer_state.palette = (attr2 >> 12) + 16;

      if(drawer_state.affine) {
        oam_fetch.step = 2;
      } else {
        drawer_state.matrix[0] = 0x100;
        drawer_state.matrix[1] = 0;
        drawer_state.matrix[2] = 0;
        drawer_state.matrix[3] = 0x100;

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

  if(!drawer_state.affine || !sprite.oam_fetch.delay_wait) {
    const int pixels = drawer_state.affine ? 1 : 2;

    sprite.timestamp_vram_access = sprite.timestamp_init + sprite.cycle;

    for(int i = 0; i < pixels; i++) {
      // @todo
      drawer_state.local_x++;
    }

    if(drawer_state.local_x >= drawer_state.half_width) {
      sprite.drawing = false;
    }
  }
}

} // namespace nba::core
