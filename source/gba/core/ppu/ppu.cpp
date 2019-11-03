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
#include <cstring>

#include "ppu.hpp"
#include "../cpu.hpp"

using namespace GameBoyAdvance;

constexpr std::uint16_t PPU::s_color_transparent;
constexpr int PPU::s_wait_cycles[3];

PPU::PPU(CPU* cpu) 
  : cpu(cpu)
  , pram(cpu->memory.pram)
  , vram(cpu->memory.vram)
  , oam(cpu->memory.oam)
{
  InitBlendTable();
  Reset();
}

void PPU::Reset() {
  mmio.dispcnt.Reset();
  mmio.dispstat.Reset();
  mmio.vcount = 0;

  for (int i = 0; i < 4; i++) {
    mmio.bgcnt[i].Reset();
    mmio.bghofs[i] = 0;
    mmio.bgvofs[i] = 0;
  }
  
  for (int i = 0; i < 2; i++) {
    mmio.bgx[i].Reset();
    mmio.bgy[i].Reset();
    mmio.bgpa[i] = 0;
    mmio.bgpb[i] = 0;
    mmio.bgpc[i] = 0;
    mmio.bgpd[i] = 0;
  }

  mmio.eva = 0;
  mmio.evb = 0;
  mmio.evy = 0;
  mmio.bldcnt.Reset();
  
  event.countdown = 0;
  SetNextEvent(Phase::SCANLINE);
}

void PPU::InitBlendTable() {
  for (int color0 = 0; color0 <= 31; color0++)
  for (int color1 = 0; color1 <= 31; color1++)
  for (int factor0 = 0; factor0 <= 16; factor0++)
  for (int factor1 = 0; factor1 <= 16; factor1++) {
    int result = (color0 * factor0 + color1 * factor1) >> 4;

    blend_table[factor0][factor1][color0][color1] = std::min<std::uint8_t>(result, 31);
  }
}

auto PPU::ConvertColor(std::uint16_t color) -> std::uint32_t {
  int r = (color >>  0) & 0x1F;
  int g = (color >>  5) & 0x1F;
  int b = (color >> 10) & 0x1F;

  return r << 19 |
         g << 11 |
         b <<  3 |
         0xFF000000;
}

void PPU::RenderScanline() {
  std::uint16_t  vcount = mmio.vcount;
  std::uint32_t* line = &output[vcount * 240];

  if (mmio.dispcnt.forced_blank) {
    for (int x = 0; x < 240; x++)
      line[x] = ConvertColor(0x7FFF);
  } else {
    std::uint16_t backdrop = ReadPalette(0, 0);
    std::uint32_t fill = backdrop * 0x00010001;

    /* Reset scanline buffers. */
    for (int i = 0; i < 240; i++) {
      ((std::uint32_t*)pixel)[i] = fill;
    }
    for (int i = 0; i < 60; i++) {
      ((std::uint32_t*)obj_attr)[i] = 0;
    }
    for (int i = 0; i < 120; i++) {
      ((std::uint32_t*)priority)[i] = 0x04040404;
      ((std::uint32_t*)layer)[i] = 0x05050505;
    }
    
    /* TODO: how does HW behave when we select mode 6 or 7? */
    switch (mmio.dispcnt.mode) {
      case 0: {
        for (int i = 3; i >= 0; i--) {
          if (mmio.dispcnt.enable[i])
            RenderLayerText(i);
        }
        if (mmio.dispcnt.enable[4])
          RenderLayerOAM();
        break;
      }
      case 1:
        if (mmio.dispcnt.enable[0])
          RenderLayerText(0);
        if (mmio.dispcnt.enable[1])
          RenderLayerText(1);
        if (mmio.dispcnt.enable[2])
          RenderLayerAffine(0);
        if (mmio.dispcnt.enable[4])
          RenderLayerOAM();
        break;
      case 2:
        if (mmio.dispcnt.enable[2])
          RenderLayerAffine(0);
        if (mmio.dispcnt.enable[3])
          RenderLayerAffine(1);
        if (mmio.dispcnt.enable[4])
          RenderLayerOAM();
        break;
      case 3:
        break;
      case 4: {
        int frame = mmio.dispcnt.frame * 0xA000;
        int offset = frame + vcount * 240;
        for (int x = 0; x < 240; x++) {
          DrawPixel(x, 2, mmio.bgcnt[2].priority, ReadPalette(0, cpu->memory.vram[offset + x]));
        }
        break;
      }
      case 5:
        break;
    }

    auto& bldcnt = mmio.bldcnt;
        
    for (int x = 0; x < 240; x++) {
      auto sfx = bldcnt.sfx;
      int layer1 = layer[0][x];
      int layer2 = layer[1][x];

      bool is_alpha_obj = (layer[0][x] == 4) && (obj_attr[x] & OBJ_IS_ALPHA);         
      bool is_sfx_1 = bldcnt.targets[0][layer1] || is_alpha_obj;
      bool is_sfx_2 = bldcnt.targets[1][layer2];

      if (is_alpha_obj && is_sfx_2) {
        sfx = BlendControl::Effect::SFX_BLEND;
      }

      if (sfx != BlendControl::Effect::SFX_NONE && is_sfx_1 &&
          (is_sfx_2 || sfx != BlendControl::Effect::SFX_BLEND)) {
        Blend(pixel[0][x], pixel[1][x], sfx);
      }
    }

    for (int x = 0; x < 240; x++) {
      line[x] = ConvertColor(pixel[0][x]);
    }
  }
}

void PPU::Blend(std::uint16_t& target1, std::uint16_t target2, BlendControl::Effect sfx) {
  int r1 = (target1 >>  0) & 0x1F;
  int g1 = (target1 >>  5) & 0x1F;
  int b1 = (target1 >> 10) & 0x1F;

  switch (sfx) {
  case BlendControl::Effect::SFX_BLEND: {
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
  case BlendControl::Effect::SFX_BRIGHTEN: {
    int evy = std::min<int>(16, mmio.evy);

    r1 = blend_table[16 - evy][evy][r1][31];
    g1 = blend_table[16 - evy][evy][g1][31];
    b1 = blend_table[16 - evy][evy][b1][31];
    break;
  }
  case BlendControl::Effect::SFX_DARKEN: {
    int evy = std::min<int>(16, mmio.evy);

    r1 = blend_table[16 - evy][evy][r1][0];
    g1 = blend_table[16 - evy][evy][g1][0];
    b1 = blend_table[16 - evy][evy][b1][0];
    break;
  }
  }

  target1 = r1 | (g1 << 5) | (b1 << 10);
}