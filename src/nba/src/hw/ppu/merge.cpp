/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <algorithm>

#include "ppu.hpp"

namespace nba::core {

static u32 RGB555(u16 rgb555) {
  const uint r = (rgb555 >>  0) & 31U;
  const uint g = (rgb555 >>  5) & 31U;
  const uint b = (rgb555 >> 10) & 31U;

  return 0xFF000000 | r << 19 | g << 11 | b << 3;
}

void PPU::InitMerge() {
  const u64 timestamp_now = scheduler.GetTimestampNow();
  
  merge.timestamp_init = timestamp_now;
  merge.timestamp_last_sync = timestamp_now;
  merge.cycle = 0U;
  merge.mosaic_x[0] = 0U;
  merge.mosaic_x[1] = 0U;
}

void PPU::DrawMerge() {
  const u64 timestamp_now = scheduler.GetTimestampNow();
  
  const int cycles = (int)(timestamp_now - merge.timestamp_last_sync);

  if(cycles == 0 || merge.cycle >= k_bg_cycle_limit) {
    return;
  }

  // @todo: possibly template this based on IO configuration
  DrawMergeImpl(cycles);

  merge.timestamp_last_sync = timestamp_now;
}

void PPU::DrawMergeImpl(int cycles) {
  static constexpr int k_min_max_bg[8][2] {
    {0,  3}, // Mode 0 (BG0 - BG3 text-mode)
    {0,  2}, // Mode 1 (BG0 - BG1 text-mode, BG2 affine)
    {2,  3}, // Mode 2 (BG2 - BG3 affine)
    {2,  2}, // Mode 3 (BG2 240x160 65526-color bitmap)
    {2,  2}, // Mode 4 (BG2 240x160 256-color bitmap, double-buffered)
    {2,  2}, // Mode 5 (BG2 160x128 65536-color bitmap, double-buffered)
    {0, -1}, // Mode 6 (invalid)
    {0, -1}, // Mode 7 (invalid)
  };

  const int mode = mmio.dispcnt.mode;

  const int min_bg = k_min_max_bg[mode][0];
  const int max_bg = k_min_max_bg[mode][1];

  // Enabled BGs sorted from highest to lowest priority.
  int bg_list[4];
  int bg_count = 0;

  for(int priority = 0; priority <= 3; priority++) {
    for(int id = min_bg; id <= max_bg; id++) {
      if(mmio.bgcnt[id].priority == priority &&
         mmio.enable_bg[0][id] &&
         mmio.dispcnt.enable[id]) {
        bg_list[bg_count++] = id;
      }
    }
  }

  // @todo: should the OBJWIN be disabled if sprites are disabled?
  const bool enable_win0 = mmio.dispcnt.enable[ENABLE_WIN0];
  const bool enable_win1 = mmio.dispcnt.enable[ENABLE_WIN1];
  const bool enable_objwin = mmio.dispcnt.enable[ENABLE_OBJWIN];

  const bool have_windows = enable_win0 || enable_win1 || enable_objwin;

  const int* win_layer_enable; // @todo: use bool

  for(int i = 0; i < cycles; i++) {
    const int cycle = (int)merge.cycle - 46;

    if(cycle >= 0 && (cycle & 3) == 0) {
      const uint x = (uint)cycle >> 2;

      if(have_windows) {
        // @todo: this probably can be optimized a bit
        if(enable_win0 && window.buffer[x][0]) {
          win_layer_enable = mmio.winin.enable[0];
        } else if(enable_win1 && window.buffer[x][1]) {
          win_layer_enable = mmio.winin.enable[1];
        } else if(enable_objwin && sprite.buffer_rd[x].window) {
          win_layer_enable = mmio.winout.enable[1];
        } else {
          win_layer_enable = mmio.winout.enable[0];
        }
      }

      int layers[2] {LAYER_BD, LAYER_BD};
      u32 colors[2] {0U, 0U};
      uint priorities[2] {3U, 3U};

      int bg_list_index = 0;

      // @todo: avoid extracting the top two layers in cases where it is not necessary.
      for(int j = 0; j < 2; j++) {
        while(bg_list_index < bg_count) {
          const int bg_id = bg_list[bg_list_index];

          bg_list_index++;

          if(!have_windows || win_layer_enable[bg_id]) {
            const auto& bgcnt = mmio.bgcnt[bg_id];
            const uint mx = x - (bgcnt.mosaic_enable ? merge.mosaic_x[0] : 0U);
            const u32 bg_color = bg.buffer[mx][bg_id];

            if(bg_color != 0U) {
              layers[j] = bg_id;
              colors[j] = bg_color;
              priorities[j] = (uint)bgcnt.priority;
              break;
            }
          }
        }
      }

      bool is_alpha_obj = false;

      if(mmio.dispcnt.enable[LAYER_OBJ] && (!have_windows || win_layer_enable[LAYER_OBJ])) {
        auto pixel = sprite.buffer_rd[x];

        if(pixel.mosaic) pixel = sprite.buffer_rd[x - merge.mosaic_x[1]];

        if(pixel.color != 0U) {
          if(pixel.priority <= priorities[0]) {
            // We do not care about the priority at this point, so we do not update it.
            layers[1] = layers[0];
            colors[1] = colors[0];
            layers[0] = LAYER_OBJ;
            colors[0] = pixel.color | 256U;

            is_alpha_obj = pixel.alpha;
          } else if(pixel.priority <= priorities[1]) {
            // We do not care about the priority at this point, so we do not update it.
            layers[1] = LAYER_OBJ;
            colors[1] = pixel.color | 256U;
          }
        }
      }

      // @todo: make it clear what the meaning of 0x8000'0000 is.
      if((colors[0] & 0x8000'0000) == 0) {
        colors[0] = FetchPRAM<u16>(merge.cycle, colors[0] << 1);
      }

      // @todo: split into two cycles here

      // @todo: make it clear what the meaning of 0x8000'0000 is.
      // @todo: do not make this fetch unless it is necessary.
      if((colors[1] & 0x8000'0000) == 0) {
        colors[1] = FetchPRAM<u16>(merge.cycle, colors[1] << 1);
      }

      const bool have_src = mmio.bldcnt.targets[1][layers[1]];

      if(is_alpha_obj && have_src) {
        colors[0] = Blend(colors[0], colors[1], mmio.eva, mmio.evb);
      } else if(!have_windows || win_layer_enable[LAYER_SFX]) {
        const bool have_dst = mmio.bldcnt.targets[0][layers[0]];

        switch(mmio.bldcnt.sfx) {
          case BlendControl::SFX_BLEND: {
            if(have_dst && have_src) {
              colors[0] = Blend(colors[0], colors[1], mmio.eva, mmio.evb);
            }
            break;
          }
          case BlendControl::SFX_BRIGHTEN: {
            if(have_dst) {
              colors[0] = Brighten(colors[0], mmio.evy);
            }
            break;
          }
          case BlendControl::SFX_DARKEN: {
            if(have_dst) {
              colors[0] = Darken(colors[0], mmio.evy);
            }
            break;
          }
        }
      }

      output[frame][mmio.vcount * 240 + x] = RGB555(colors[0]);

      if(++merge.mosaic_x[0] == (uint)mmio.mosaic.bg.size_x) {
        merge.mosaic_x[0] = 0U;
      }

      if(++merge.mosaic_x[1] == (uint)mmio.mosaic.obj.size_x) {
        merge.mosaic_x[1] = 0U;
      }
    }

    if(++merge.cycle == k_bg_cycle_limit) {
      break;
    }
  }
}

auto PPU::Blend(u16 color_a, u16 color_b, int eva, int evb) -> u16 {
  const int r_a =  (color_a >>  0) & 31;
  const int g_a = ((color_a >>  4) & 62) | (color_a >> 15 << 5);
  const int b_a =  (color_a >> 10) & 31;

  const int r_b =  (color_b >>  0) & 31;
  const int g_b = ((color_b >>  4) & 62) | (color_b >> 15 << 5);
  const int b_b =  (color_b >> 10) & 31;

  eva = std::min<int>(16, eva);
  evb = std::min<int>(16, evb);

  const int r = std::min<u8>((r_a * eva + r_b * evb + 8) >> 4, 31);
  const int g = std::min<u8>((g_a * eva + g_b * evb + 8) >> 4, 63) >> 1;
  const int b = std::min<u8>((b_a * eva + b_b * evb + 8) >> 4, 31);

  return (u16)((b << 10) | (g << 5) | r);
}

auto PPU::Brighten(u16 color, int evy) -> u16 {
  evy = std::min<int>(16, evy);

  int r =  (color >>  0) & 31;
  int g = ((color >>  4) & 62) | (color >> 15 << 5);
  int b =  (color >> 10) & 31;

  r += ((31 - r) * evy + 8) >> 4;
  g += ((63 - g) * evy + 8) >> 4;
  b += ((31 - b) * evy + 8) >> 4;

  g >>= 1;
  
  return (u16)((b << 10) | (g << 5) | r);
}

auto PPU::Darken(u16 color, int evy) -> u16 {
  evy = std::min<int>(16, evy);

  int r =  (color >>  0) & 31;
  int g = ((color >>  4) & 62) | (color >> 15 << 5);
  int b =  (color >> 10) & 31;

  r -= (r * evy + 7) >> 4;
  g -= (g * evy + 7) >> 4;
  b -= (b * evy + 7) >> 4;

  g >>= 1;

  return (u16)((b << 10) | (g << 5) | r);
}

} // namespace nba::core
