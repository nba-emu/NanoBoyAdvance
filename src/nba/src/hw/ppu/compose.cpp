/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <algorithm>

#include "hw/ppu/ppu.hpp"

namespace nba::core {

using BlendMode = BlendControl::Effect;

auto PPU::ConvertColor(u16 color) -> u32 {
  int r = (color >>  0) & 0x1F;
  int g = (color >>  5) & 0x1F;
  int b = (color >> 10) & 0x1F;

  return r << 19 |
         g << 11 |
         b <<  3 |
         0xFF000000;
}

void PPU::RenderScanline() {
  u16  vcount = mmio.vcount;
  u32* line = &output[vcount * 240];

  if (mmio.dispcnt.forced_blank) {
    for (int x = 0; x < 240; x++) {
      line[x] = ConvertColor(0x7FFF);
    }
    return;
  }

  switch (mmio.dispcnt.mode) {
    // BG Mode 0 - 240x160 pixels, Text mode
    case 0: {
      for (int i = 0; i < 4; i++) {
        if (mmio.dispcnt.enable[i]) {
          RenderLayerText(i);
        }/* else if (!enable_bg[0][i]) {
          // TODO: fix this properly!
          for (int x = 0; x < 240; x++) {
            buffer_bg[i][x] = s_color_transparent;
          }
        }*/
      }
      ComposeScanline(0, 3);
      break;
    }
    // BG Mode 1 - 240x160 pixels, Text and RS mode mixed
    case 1: {
      for (int i = 0; i < 2; i++) {
        if (mmio.dispcnt.enable[i]) {
          RenderLayerText(i);
        }
      }
      if (mmio.dispcnt.enable[ENABLE_BG2]) {
        RenderLayerAffine(0);
      }
      ComposeScanline(0, 2);
      break;
    }
    // BG Mode 2 - 240x160 pixels, RS mode
    case 2: {
      for (int i = 0; i < 2; i++) {
        if (mmio.dispcnt.enable[2 + i]) {
          RenderLayerAffine(i);
        }
      }
      ComposeScanline(2, 3);
      break;
    }
    // BG Mode 3 - 240x160 pixels, 32768 colors
    case 3: {
      if (mmio.dispcnt.enable[2]) {
        RenderLayerBitmap1();
      }
      ComposeScanline(2, 2);
      break;
    }
    // BG Mode 4 - 240x160 pixels, 256 colors (out of 32768 colors)
    case 4: {
      if (mmio.dispcnt.enable[2]) {
        RenderLayerBitmap2();
      }
      ComposeScanline(2, 2);
      break;
    }
    // BG Mode 5 - 160x128 pixels, 32768 colors
    case 5: {
      if (mmio.dispcnt.enable[2]) {
        RenderLayerBitmap3();
      }
      ComposeScanline(2, 2);
      break;
    }
    // BG Modes 6/7 (invalid) - output backdrop color
    case 6:
    case 7: {
      // TODO: do OBJs still work in this mode?
      u32 backdrop = ConvertColor(ReadPalette(0, 0));
      for (int x = 0; x < 240; x++) {
        line[x] = backdrop;
      }
      break;
    }
  }
}

template<bool window, bool blending>
void PPU::ComposeScanlineTmpl(int bg_min, int bg_max) {
  u32* line = &output[mmio.vcount * 240];
  u16 backdrop = ReadPalette(0, 0);

  auto const& dispcnt = mmio.dispcnt;
  auto const& bgcnt = mmio.bgcnt;
  auto const& winin = mmio.winin;
  auto const& winout = mmio.winout;

  int bg_list[4];
  int bg_count = 0;

  // Sort enabled backgrounds by their respective priority in ascending order.
  for (int prio = 3; prio >= 0; prio--) {
    for (int bg = bg_max; bg >= bg_min; bg--) {
      if (enable_bg[0][bg] && mmio.dispcnt.enable[bg] && bgcnt[bg].priority == prio) {
        bg_list[bg_count++] = bg;
      }
    }
  }

  bool win0_active = false;
  bool win1_active = false;
  bool win2_active = false;

  const int* win_layer_enable;
  
  if constexpr (window) {
    win0_active = dispcnt.enable[ENABLE_WIN0] && window_scanline_enable[0];
    win1_active = dispcnt.enable[ENABLE_WIN1] && window_scanline_enable[1];
    win2_active = dispcnt.enable[ENABLE_OBJWIN];
  }

  int prio[2];
  int layer[2];
  u16 pixel[2];

  for (int x = 0; x < 240; x++) {
    if constexpr (window) {
      // Determine the window with the highest priority for this pixel.
      if (win0_active && buffer_win[0][x]) {
        win_layer_enable = winin.enable[0];
      } else if (win1_active && buffer_win[1][x]) {
        win_layer_enable = winin.enable[1];
      } else if (win2_active && buffer_obj[x].window) {
        win_layer_enable = winout.enable[1];
      } else {
        win_layer_enable = winout.enable[0];
      }
    }

    if constexpr (blending) {
      bool is_alpha_obj = false;

      prio[0] = 4;
      prio[1] = 4;
      layer[0] = LAYER_BD;
      layer[1] = LAYER_BD;
      
      // Find up to two top-most visible background pixels.
      for (int i = 0; i < bg_count; i++) {
        int bg = bg_list[i];

        if (!window || win_layer_enable[bg]) {
          auto pixel_new = buffer_bg[bg][x];
          if (pixel_new != s_color_transparent) {
            layer[1] = layer[0];
            layer[0] = bg;
            prio[1] = prio[0];
            prio[0] = bgcnt[bg].priority;
          }
        }
      }

      /* Check if a OBJ pixel takes priority over one of the two
       * top-most background pixels and insert it accordingly.
       */
      if ((!window || win_layer_enable[LAYER_OBJ]) &&
          dispcnt.enable[ENABLE_OBJ] &&
          buffer_obj[x].color != s_color_transparent) {
        int priority = buffer_obj[x].priority;

        if (priority <= prio[0]) {
          layer[1] = layer[0];
          layer[0] = LAYER_OBJ;
          is_alpha_obj = buffer_obj[x].alpha;
        } else if (priority <= prio[1]) {
          layer[1] = LAYER_OBJ;
        }
      }

      // Map layer numbers to pixels.
      for (int i = 0; i < 2; i++) {
        int _layer = layer[i];
        switch (_layer) {
          case 0 ... 3:
            pixel[i] = buffer_bg[_layer][x];
            break;
          case 4:
            pixel[i] = buffer_obj[x].color;
            break;
          case 5:
            pixel[i] = backdrop;
            break;
        }
      }

      if (!window || win_layer_enable[LAYER_SFX] || is_alpha_obj) {
        auto blend_mode = mmio.bldcnt.sfx;
        bool have_dst = mmio.bldcnt.targets[0][layer[0]];
        bool have_src = mmio.bldcnt.targets[1][layer[1]];

        if (is_alpha_obj && have_src) {
          Blend(pixel[0], pixel[1], BlendMode::SFX_BLEND);
        } else if (have_dst && blend_mode != BlendMode::SFX_NONE && (have_src || blend_mode != BlendMode::SFX_BLEND)) {
          Blend(pixel[0], pixel[1], blend_mode);
        }
      }
    } else {
      pixel[0] = backdrop;
      prio[0] = 4;
      
      // Find the top-most visible background pixel.
      if (bg_count != 0) {
        for (int i = bg_count - 1; i >= 0; i--) {
          int bg = bg_list[i];

          if (!window || win_layer_enable[bg]) {
            u16 pixel_new = buffer_bg[bg][x];
            if (pixel_new != s_color_transparent) {
              pixel[0] = pixel_new;
              prio[0] = bgcnt[bg].priority;
              break;
            }   
          }
        }
      }

      // Check if a OBJ pixel takes priority over the top-most background pixel.
      if ((!window || win_layer_enable[LAYER_OBJ]) &&
          dispcnt.enable[ENABLE_OBJ] &&
          buffer_obj[x].color != s_color_transparent &&
          buffer_obj[x].priority <= prio[0]) {
        pixel[0] = buffer_obj[x].color;
      }
    }

    line[x] = ConvertColor(pixel[0]);
  }
}

void PPU::ComposeScanline(int bg_min, int bg_max) {
  auto const& dispcnt = mmio.dispcnt;

  int key = 0;

  if (dispcnt.enable[ENABLE_WIN0] ||
      dispcnt.enable[ENABLE_WIN1] ||
      dispcnt.enable[ENABLE_OBJWIN]) {
    key |= 1;
  }

  if (mmio.bldcnt.sfx != BlendMode::SFX_NONE || line_contains_alpha_obj) {
    key |= 2;
  }

  switch (key) {
    case 0b00:
      ComposeScanlineTmpl<false, false>(bg_min, bg_max);
      break;
    case 0b01:
      ComposeScanlineTmpl<true, false>(bg_min, bg_max);
      break;
    case 0b10:
      ComposeScanlineTmpl<false, true>(bg_min, bg_max);
      break;
    case 0b11:
      ComposeScanlineTmpl<true, true>(bg_min, bg_max);
      break;
  }
}

void PPU::Blend(u16& target1,
                u16  target2,
                BlendMode sfx) {
  int r1 = (target1 >>  0) & 0x1F;
  int g1 = (target1 >>  5) & 0x1F;
  int b1 = (target1 >> 10) & 0x1F;

  switch (sfx) {
    case BlendMode::SFX_BLEND: {
      int eva = std::min<int>(16, mmio.eva);
      int evb = std::min<int>(16, mmio.evb);

      int r2 = (target2 >>  0) & 0x1F;
      int g2 = (target2 >>  5) & 0x1F;
      int b2 = (target2 >> 10) & 0x1F;

      r1 = std::min<u8>((r1 * eva + r2 * evb) >> 4, 31);
      g1 = std::min<u8>((g1 * eva + g2 * evb) >> 4, 31);
      b1 = std::min<u8>((b1 * eva + b2 * evb) >> 4, 31);
      break;
    }
    case BlendMode::SFX_BRIGHTEN: {
      int evy = std::min<int>(16, mmio.evy);
      
      r1 += ((31 - r1) * evy) >> 4;
      g1 += ((31 - g1) * evy) >> 4;
      b1 += ((31 - b1) * evy) >> 4;
      break;
    }
    case BlendMode::SFX_DARKEN: {
      int evy = std::min<int>(16, mmio.evy);
      
      r1 -= (r1 * evy) >> 4;
      g1 -= (g1 * evy) >> 4;
      b1 -= (b1 * evy) >> 4;
      break;
    }
  }

  target1 = r1 | (g1 << 5) | (b1 << 10);
}

} // namespace nba::core
