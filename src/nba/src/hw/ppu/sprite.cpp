/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "ppu.hpp"

namespace nba::core {

static const int s_obj_size[4][4][2] = {
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

void PPU::InitSprite() {
  const uint vcount = mmio.vcount;
  const u64 timestamp_now = scheduler.GetTimestampNow();

  sprite.timestamp_init = timestamp_now;
  sprite.timestamp_last_sync = timestamp_now;
  sprite.cycle = 0U;
  sprite.vcount = vcount;
  sprite.index = 0U;

  sprite.oam_fetch_state = OAMFetchState::Attribute01;
  sprite.oam_access_wait = 0;
  sprite.first_vram_access_cycle = false;

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
  const uint cycle_limit = mmio.dispcnt.hblank_oam_access ? 964U : 1232U;

  for(int i = 0; i < cycles; i++) {
    const uint cycle = sprite.cycle;

    if((cycle & 1U) == 0U) {
      if(sprite.oam_access_wait == 0 || sprite.first_vram_access_cycle) {
        auto& state = sprite.state[(sprite.index & 1U) ^ 1];

        sprite.first_vram_access_cycle = false;

        switch(sprite.oam_fetch_state) {
          case OAMFetchState::Attribute01: {
            // @todo: solve this in a cleaner way
            if(sprite.index >= 128U) {
              break;
            }

            const u32 attr01 = read<u32>(oam, sprite.index * 8U);

            // @todo: optimise VRAM access stall emulation
            sprite.timestamp_oam_access = sprite.timestamp_init + cycle;

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

                // @todo: replace the LUT with something more sensible.
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
                  state.x = center_x;
                  state.y = center_y;
                  state.width = width;
                  state.height = height;
                  state.half_width = half_width;
                  state.half_height = half_height;
                  state.mode = mode;
                  state.affine = affine;
                  state.local_x = -half_width;
                  state.local_y = local_y;

                  state.transform_id = ((attr01 >> 25) & 31) << 5;
                  state.flip_h = !affine && (attr01 & (1 << 28));
                  state.flip_v = !affine && (attr01 & (1 << 29));
                  state.is_256 = (attr01 >> 13) & 1;
                }
              }
            }

            if(active) {
              sprite.oam_fetch_state = OAMFetchState::Attribute2;
            } else {
              sprite.index++;
            }
            break;
          }
          case OAMFetchState::Attribute2: {
            const u16 attr2 = read<u16>(oam, sprite.index * 8U + 4U);

            // @todo: optimise VRAM access stall emulation
            sprite.timestamp_oam_access = sprite.timestamp_init + cycle;

            // fmt::print("fetch Attr2 @ {}\n", sprite.timestamp_oam_access);

            state.tile_number = attr2 & 0x3FF;
            state.priority = (attr2 >> 10) & 3;
            state.palette = (attr2 >> 12) + 16;

            if(state.affine) {
              sprite.oam_fetch_state = OAMFetchState::PA;
            } else {
              state.transform[0] = 0x100;
              state.transform[1] = 0;
              state.transform[2] = 0;
              state.transform[3] = 0x100;

              // @todo
              sprite.oam_fetch_state = OAMFetchState::Attribute01;
              sprite.oam_access_wait = state.half_width - 2;
              sprite.first_vram_access_cycle = true;
              sprite.index++;
            }
            break;
          }
          case OAMFetchState::PA: {
            state.transform[0] = read<s16>(oam, state.transform_id + 0x06);

            // @todo: optimise VRAM access stall emulation
            sprite.timestamp_oam_access = sprite.timestamp_init + cycle;

            sprite.oam_fetch_state = OAMFetchState::PB;
            break;
          }
          case OAMFetchState::PB: {
            state.transform[1] = read<s16>(oam, state.transform_id + 0x0E);

            // @todo: optimise VRAM access stall emulation
            sprite.timestamp_oam_access = sprite.timestamp_init + cycle;

            sprite.oam_fetch_state = OAMFetchState::PC;
            break;
          }
          case OAMFetchState::PC: {
            state.transform[2] = read<s16>(oam, state.transform_id + 0x16);

            // @todo: optimise VRAM access stall emulation
            sprite.timestamp_oam_access = sprite.timestamp_init + cycle;

            sprite.oam_fetch_state = OAMFetchState::PD;
            break;
          }
          case OAMFetchState::PD: {
            state.transform[3] = read<s16>(oam, state.transform_id + 0x1E);

            // @todo: optimise VRAM access stall emulation
            sprite.timestamp_oam_access = sprite.timestamp_init + cycle;

            // @todo
            sprite.oam_fetch_state = OAMFetchState::Attribute01;
            sprite.oam_access_wait = state.half_width * 2 - 1;
            sprite.first_vram_access_cycle = true;
            sprite.index++;
            break;
          }
        }
      } else {
        sprite.oam_access_wait--;
      }
    }

    if(++sprite.cycle == cycle_limit) {
      break;
    }
  }
}

} // namespace nba::core
