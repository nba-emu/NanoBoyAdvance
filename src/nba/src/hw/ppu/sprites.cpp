/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <optional>

#include "hw/ppu/ppu.hpp"

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

void PPU::InitOBJ() {
  obj.engaged = true;
  obj.hcounter = 0;
  obj.index = 0;
  obj.oam_fetch_state = Attr01;
  obj.rendering = false;
  obj.state_rd = 0;
  obj.state_wr = 1;
  obj.oam_access_wait = 0;
  obj.first_vram_access_cycle = false;

  auto buffer = buffer_obj[~mmio.vcount & 1];

  for (int x = 0; x < 240; x++) {
    buffer[x].priority = 4;
    buffer[x].color = 0x8000'0000;
    buffer[x].alpha = 0;
    buffer[x].window = 0;
    buffer[x].mosaic = 0;
  }
}

void PPU::SyncOBJ(int cycles) {
  const int RENDER_DELAY = 42;

  int hcounter = obj.hcounter;
  int hcounter_current = hcounter + cycles;
  int hcounter_target = std::min(hcounter_current, 1210 /*!TODO!*/);
  std::optional<int> last_oam_access;
  std::optional<int> last_vram_access;

  obj.hcounter = hcounter_current;

  while (hcounter < hcounter_target) {
    // only do anything every two cycles, yes...
    if ((hcounter & 1) || hcounter < RENDER_DELAY) {
      hcounter++;
      continue;
    }

    if (obj.rendering) {
      auto& state = obj.state[obj.state_rd];

      if (!state.affine || !obj.first_vram_access_cycle) {
        int pixel_step = state.affine ? 1 : 2;

        int width = state.width;
        int height = state.height;
        int mode = state.mode;
        int local_y = state.local_y;
        int tile_number = state.tile_number;
        int priority = state.priority;
        int palette = state.palette;
        bool flip_h = state.flip_h;
        bool flip_v = state.flip_v;
        bool is_256 = state.is_256;

        u32 tile_base = 0x10000; // @todo: is this ever modified?
        bool bitmap_mode = dispcnt_mode >= 3;

        for (int i = 0; i < pixel_step; i++) {
          int local_x = state.local_x + i;
          int global_x = state.x + local_x;

          if (global_x < 0 || global_x >= 240) {
            continue;
          }

          // @todo: this can largely be precomputed
          int tex_x = ((state.transform[0] * local_x + state.transform[1] * local_y) >> 8) + (width  / 2);
          int tex_y = ((state.transform[2] * local_x + state.transform[3] * local_y) >> 8) + (height / 2);

          if (tex_x >= width || tex_y >= height || tex_x < 0 || tex_y < 0) {
            continue;
          }

          // @todo: this can be optimized with a XOR-operation
          if (flip_h) tex_x = width  - tex_x - 1;
          if (flip_v) tex_y = height - tex_y - 1;

          // @todo: this can be optimized
          int tile_x  = tex_x % 8;
          int tile_y  = tex_y % 8;
          int block_x = tex_x / 8;
          int block_y = tex_y / 8;

          int tile_num; // @todo: this name is confusing with another variable called tile_number
          u32 pixel;

          if (is_256) {
            block_x *= 2;

            // @todo: this flag probably should be latched
            if (mmio.dispcnt.oam_mapping_1d) {
              tile_num = (tile_number + block_y * (width >> 2) + block_x) & 0x3FF;
            } else {
              tile_num = ((tile_number + (block_y * 32)) & 0x3E0) | (((tile_number & ~1) + block_x) & 0x1F);
            }

            // @todo: can this be solved more elegantly?
            if (bitmap_mode && tile_num < 512) {
              continue;
            }

            pixel = DecodeTilePixel8BPP(tile_base + tile_num * 32, tile_x, tile_y, true);
          } else {
            // @todo: this flag probably should be latched
            if (mmio.dispcnt.oam_mapping_1d) {
              tile_num = (tile_number + block_y * (width >> 3) + block_x) & 0x3FF;
            } else {
              tile_num = ((tile_number + (block_y * 32)) & 0x3E0) | ((tile_number + block_x) & 0x1F);
            }

            // @todo: can this be solved more elegantly?
            if (bitmap_mode && tile_num < 512) {
              continue;
            }

            pixel = DecodeTilePixel4BPP(tile_base + tile_num * 32, palette, tile_x, tile_y);
          }

          auto& point = buffer_obj[~mmio.vcount & 1][global_x];
          bool opaque = pixel != 0x8000'0000;

          if (mode == OBJ_WINDOW) {
            if (opaque) point.window = 1;
          } else if (priority < point.priority || point.color == 0x8000'0000) {
            if (opaque) {
              point.color = pixel;
              point.alpha = (mode == OBJ_SEMI) ? 1 : 0;
            }

            // point.mosaic = mosaic ? 1 : 0;
            point.priority = priority;
          }
        }

        state.local_x += pixel_step;

        last_vram_access = hcounter;

        if (state.local_x >= state.half_width) {
          obj.rendering = false;
        }
      }
    }

    if (obj.oam_access_wait == 0 || obj.first_vram_access_cycle) {
      auto& state = obj.state[obj.state_wr];

      obj.first_vram_access_cycle = false;

      switch (obj.oam_fetch_state) {
        case OAMFetchState::Attr01: {
          // @todo: better solve this:
          if (obj.index >= 128) {
            break;
          }

          u32 attr01 = read<u32>(oam, obj.index * 8);

          last_oam_access = hcounter;

          bool active = false;

          if ((attr01 & 0x300) != 0x200) {
            int mode = (attr01 >> 10) & 3;

            // @todo: how does HW handle OBJs in 'prohibited' mode?
            if (mode != OBJ_PROHIBITED) {
              s32 x = (attr01 >> 16) & 0x1FF;
              s32 y =  attr01 & 0xFF;

              if (x >= 240) x -= 512;
              if (y >= 160) y -= 256;

              int shape = (attr01 >> 14) & 3;
              int size  =  attr01 >> 30;

              int width  = s_obj_size[shape][size][0];
              int height = s_obj_size[shape][size][1];

              int half_width  = width  >> 1;
              int half_height = height >> 1;

              bool affine = attr01 & 0x100;

              if (affine) {
                bool double_size = attr01 & 0x200;

                if (double_size) {
                  half_width  *= 2;
                  half_height *= 2;
                }
              }

              int center_x = x + half_width;
              int center_y = y + half_height;
              int local_y = mmio.vcount - center_y; // @todo: VCOUNT is not correct

              if (local_y >= -half_height && local_y < half_height) {
                active = true;

                // @todo: sprite mosaic
                state.x = center_x;//x;
                state.y = center_y;//y;
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

          if (active) {
            // @todo: are PA-PD fetched before or after Attr2???
            obj.oam_fetch_state = OAMFetchState::Attr2;
          } else {
            obj.index++;
          }
          break;
        }
        case OAMFetchState::Attr2: {
          u16 attr2 = read<u16>(oam, obj.index * 8 + 4);

          last_oam_access = hcounter;

          state.tile_number = attr2 & 0x3FF;
          state.priority = (attr2 >> 10) & 3;
          state.palette  = (attr2 >> 12) + 16;

          // @todo: transition to PA if sprite is affine
          if (state.affine) {
            obj.oam_fetch_state = OAMFetchState::PA;
          } else {
            state.transform[0] = 0x100;
            state.transform[1] = 0;
            state.transform[2] = 0;
            state.transform[3] = 0x100;

            // @todo: this code is redundant
            obj.rendering = true;
            obj.state_rd ^= 1;
            obj.state_wr ^= 1;
            obj.oam_fetch_state = OAMFetchState::Attr01;
            obj.index++;

            obj.oam_access_wait = state.half_width - 2;
            obj.first_vram_access_cycle = true;
          }
          break;
        }
        case OAMFetchState::PA: {
          state.transform[0] = read<s16>(oam, state.transform_id + 0x06);
          last_oam_access = hcounter;
          obj.oam_fetch_state = OAMFetchState::PB;
          break;
        }
        case OAMFetchState::PB: {
          state.transform[1] = read<s16>(oam, state.transform_id + 0x0E);
          last_oam_access = hcounter;
          obj.oam_fetch_state = OAMFetchState::PC;
          break;
        }
        case OAMFetchState::PC: {
          state.transform[2] = read<s16>(oam, state.transform_id + 0x16);
          last_oam_access = hcounter;
          obj.oam_fetch_state = OAMFetchState::PD;
          break;
        }
        case OAMFetchState::PD: {
          state.transform[3] = read<s16>(oam, state.transform_id + 0x1E);
          last_oam_access = hcounter;

          // @todo: this code is redundant
          obj.rendering = true;
          obj.state_rd ^= 1;
          obj.state_wr ^= 1;
          obj.oam_fetch_state = OAMFetchState::Attr01;
          obj.index++;

          obj.oam_access_wait = state.half_width * 2 - 1;
          obj.first_vram_access_cycle = true;
          break;
        }
      }
    } else {
      obj.oam_access_wait--;
    }

    hcounter++;
  }

  if (last_oam_access.has_value() && last_oam_access.value() == hcounter_current - 1) {
    oam_access = true;
  }

  if (last_vram_access.has_value() && last_vram_access.value() == hcounter_current - 1) {
    vram_obj_access = true;
  }
}

void PPU::RenderLayerOAM(bool bitmap_mode, int line) {
  /*int tile_num;
  u32 pixel;
  s16 transform[4];
  int cycles = mmio.dispcnt.hblank_oam_access ? 954 : 1210;

  for (int x = 0; x < 240; x++) {
    buffer_obj[x].priority = 4;
    buffer_obj[x].color = 0x8000'0000;
    buffer_obj[x].alpha = 0;
    buffer_obj[x].window = 0;
    buffer_obj[x].mosaic = 0;
  }

  for (s32 offset = 0; offset <= 127 * 8; offset += 8) {
    if ((oam[offset + 1] & 3) == 2) {
      continue;
    }

    u16 attr0 = (oam[offset + 1] << 8) | oam[offset + 0];
    u16 attr1 = (oam[offset + 3] << 8) | oam[offset + 2];
    u16 attr2 = (oam[offset + 5] << 8) | oam[offset + 4];

    int width;
    int height;

    s32 x = attr1 & 0x1FF;
    s32 y = attr0 & 0x0FF;
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

    int center_x = x + half_width;
    int center_y = y + half_height;
    int local_y = line - center_y;

    if (local_y < -half_height || local_y >= half_height) {
      continue;
    }

    int number  =  attr2 & 0x3FF;
    int palette = (attr2 >> 12) + 16;
    int flip_h  = !affine && (attr1 & (1 << 12));
    int flip_v  = !affine && (attr1 & (1 << 13));
    int is_256  = (attr0 >> 13) & 1;

    u32 tile_base = 0x10000;

    if (mosaic) {
      local_y -= mmio.mosaic.obj._counter_y;
    }

    for (int local_x = -half_width; local_x < half_width; local_x++) {
      int global_x = local_x + center_x;

      if (global_x < 0 || global_x >= 240) {
        continue;
      }

      int tex_x = ((transform[0] * local_x + transform[1] * local_y) >> 8) + (width / 2);
      int tex_y = ((transform[2] * local_x + transform[3] * local_y) >> 8) + (height / 2);

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
        block_x *= 2;

        if (mmio.dispcnt.oam_mapping_1d) {
          tile_num = (number + block_y * (width >> 2) + block_x) & 0x3FF;
        } else {
          tile_num = ((number + (block_y * 32)) & 0x3E0) | (((number & ~1) + block_x) & 0x1F);
        }

        if (bitmap_mode && tile_num < 512) {
          continue;
        }

        pixel = DecodeTilePixel8BPP(tile_base + tile_num * 32, tile_x, tile_y, true);
      } else {
        if (mmio.dispcnt.oam_mapping_1d) {
          tile_num = (number + block_y * (width >> 3) + block_x) & 0x3FF;
        } else {
          tile_num = ((number + (block_y * 32)) & 0x3E0) | ((number + block_x) & 0x1F);
        }

        if (bitmap_mode && tile_num < 512) {
          continue;
        }

        pixel = DecodeTilePixel4BPP(tile_base + tile_num * 32, palette, tile_x, tile_y);
      }

      auto& point = buffer_obj[global_x];
      bool opaque = pixel != 0x8000'0000;

      if (mode == OBJ_WINDOW) {
        if (opaque) point.window = 1;
      } else if (prio < point.priority || point.color == 0x8000'0000) {
        if (opaque) {
          point.color = pixel;
          point.alpha = (mode == OBJ_SEMI) ? 1 : 0;
        }

        point.mosaic = mosaic ? 1 : 0;
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
  }*/
}

} // namespace nba::core
