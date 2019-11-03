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

#include <algorithm>

#include "ppu.hpp"
#include "../cpu.hpp"

using namespace GameBoyAdvance;

using BlendMode = BlendControl::Effect;

auto PPU::ConvertColor(std::uint16_t color) -> std::uint32_t {
  int r = (color >>  0) & 0x1F;
  int g = (color >>  5) & 0x1F;
  int b = (color >> 10) & 0x1F;

  return r << 19 |
         g << 11 |
         b <<  3 |
         0xFF000000;
}

void PPU::InitBlendTable() {
  for (int color0 = 0; color0 <= 31; color0++) {
    for (int color1 = 0; color1 <= 31; color1++) {
      for (int factor0 = 0; factor0 <= 16; factor0++) {
        for (int factor1 = 0; factor1 <= 16; factor1++) {
          int result = (color0 * factor0 + color1 * factor1) >> 4;

          blend_table[factor0][factor1][color0][color1] = std::min<std::uint8_t>(result, 31);
        }
      }
    }
  }
}

void PPU::RenderScanline() {
  std::uint16_t  vcount = mmio.vcount;
  std::uint32_t* line = &output[vcount * 240];

  if (mmio.dispcnt.forced_blank) {
    for (int x = 0; x < 240; x++) {
      line[x] = ConvertColor(0x7FFF);
    }
    return;
  }
  
  if (mmio.dispcnt.enable[ENABLE_WIN0]) {
    RenderWindow(0);
  }
  
  if (mmio.dispcnt.enable[ENABLE_WIN1]) {
    RenderWindow(1);
  }
  
  /* TODO: properly implement the bitmap modes. */
  switch (mmio.dispcnt.mode) {
    case 0: {
      for (int i = 0; i < 4; i++) {
        if (mmio.dispcnt.enable[i]) {
          RenderLayerText(i);
        }
      }
      if (mmio.dispcnt.enable[ENABLE_OBJ]) {
        RenderLayerOAM();
      }
      ComposeScanline(0, 3);
      break;
    }
    case 1: {
      for (int i = 0; i < 2; i++) {
        if (mmio.dispcnt.enable[i]) {
          RenderLayerText(i);
        }
      }
      if (mmio.dispcnt.enable[ENABLE_BG2]) {
        RenderLayerAffine(0);
      }
      if (mmio.dispcnt.enable[ENABLE_OBJ]) {
        RenderLayerOAM();
      }
      ComposeScanline(0, 2);
      break;
    }
    case 2: {
      for (int i = 0; i < 2; i++) {
        if (mmio.dispcnt.enable[2 + i]) {
          RenderLayerAffine(i);
        }
      }
      if (mmio.dispcnt.enable[ENABLE_OBJ]) {
        RenderLayerOAM();
      }
      ComposeScanline(0, 1);
      break;
    }
    case 4: {
      int frame = mmio.dispcnt.frame * 0xA000;
      int offset = frame + vcount * 240;
      for (int x = 0; x < 240; x++) {
        buffer_bg[2][x] = ReadPalette(0, cpu->memory.vram[offset + x]);
      }
      ComposeScanline(2, 2);
      break;
    }
  }
}

void PPU::ComposeScanline(int bg_min, int bg_max) {
  std::uint32_t* line = &output[mmio.vcount * 240];
  std::uint16_t backdrop = ReadPalette(0, 0);
  
  auto const& dispcnt = mmio.dispcnt;
  auto const& bgcnt = mmio.bgcnt;
  auto const& winin = mmio.winin;
  auto const& winout = mmio.winout;
  
  bool win0_active = dispcnt.enable[ENABLE_WIN0] && window_scanline_enable[0];
  bool win1_active = dispcnt.enable[ENABLE_WIN1] && window_scanline_enable[1];
  bool win2_active = dispcnt.enable[ENABLE_OBJWIN];
  bool no_windows  = !dispcnt.enable[ENABLE_WIN0] && 
                     !dispcnt.enable[ENABLE_WIN1] &&
                     !dispcnt.enable[ENABLE_OBJWIN];
  
  for (int x = 0; x < 240; x++) {
    int layer[2] = { LAYER_BD, LAYER_BD };
    std::uint16_t pixel[2] = { backdrop, 0 };
    
    const int* win_layer_enable = winout.enable[0];
    
    if (win0_active && buffer_win[0][x]) {
      win_layer_enable = winin.enable[0];
    } else if (win1_active && buffer_win[1][x]) {
      win_layer_enable = winin.enable[1];
    } else if (win2_active && buffer_obj[x].window) {
      win_layer_enable = winout.enable[1];
    }
    
    /* TODO: for maximum effiency maybe we can pre-sort the BGs by priority. */
    for (int prio = 3; prio >= 0; prio--) {
      for (int bg = bg_max; bg >= bg_min; bg--) {
        if (dispcnt.enable[bg] && bgcnt[bg].priority == prio && (no_windows || win_layer_enable[bg])) {
          auto pixel_new = buffer_bg[bg][x];
          if (pixel_new != s_color_transparent) {
            layer[1] = layer[0];
            layer[0] = bg;
            pixel[1] = pixel[0];
            pixel[0] = pixel_new;
          }
        }
      }
      
      if (dispcnt.enable[ENABLE_OBJ] && buffer_obj[x].priority == prio && (no_windows || win_layer_enable[LAYER_OBJ])) {
        auto pixel_new = buffer_obj[x].color;
        if (pixel_new != s_color_transparent) {
          layer[1] = layer[0];
          layer[0] = LAYER_OBJ;
          pixel[1] = pixel[0];
          pixel[0] = pixel_new;
        }
      }
    }
    
    if (no_windows || win_layer_enable[LAYER_SFX] || (layer[0] == LAYER_OBJ && buffer_obj[x].alpha)) {
      auto blend_mode = mmio.bldcnt.sfx;
      bool is_alpha_obj = layer[0] == LAYER_OBJ && buffer_obj[x].alpha;
      bool have_dst = mmio.bldcnt.targets[0][layer[0]] | is_alpha_obj;
      bool have_src = mmio.bldcnt.targets[1][layer[1]];

      if (is_alpha_obj && have_src) {
        blend_mode = BlendMode::SFX_BLEND;
      }

      if (blend_mode != BlendMode::SFX_NONE && have_dst &&
          (have_src || blend_mode != BlendMode::SFX_BLEND)) {
        Blend(pixel[0], pixel[1], blend_mode);
      }
    }

    line[x] = ConvertColor(pixel[0]);
  }
}

void PPU::Blend(std::uint16_t& target1,
                std::uint16_t target2,
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

      r1 = blend_table[eva][evb][r1][r2];
      g1 = blend_table[eva][evb][g1][g2];
      b1 = blend_table[eva][evb][b1][b2];
      break;
    }
    case BlendMode::SFX_BRIGHTEN: {
      int evy = std::min<int>(16, mmio.evy);

      r1 = blend_table[16 - evy][evy][r1][31];
      g1 = blend_table[16 - evy][evy][g1][31];
      b1 = blend_table[16 - evy][evy][b1][31];
      break;
    }
    case BlendMode::SFX_DARKEN: {
      int evy = std::min<int>(16, mmio.evy);

      r1 = blend_table[16 - evy][evy][r1][0];
      g1 = blend_table[16 - evy][evy][g1][0];
      b1 = blend_table[16 - evy][evy][b1][0];
      break;
    }
  }

  target1 = r1 | (g1 << 5) | (b1 << 10);
}