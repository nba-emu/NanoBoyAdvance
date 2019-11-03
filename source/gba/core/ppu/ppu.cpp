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

void PPU::RenderScanline() {
  std::uint16_t  vcount = mmio.vcount;
  std::uint32_t* line = &output[vcount * 240];

  if (mmio.dispcnt.forced_blank) {
    for (int x = 0; x < 240; x++) {
      line[x] = ConvertColor(0x7FFF);
    }
    return;
  }
  
  if (mmio.dispcnt.enable[DisplayControl::ENABLE_WIN0]) {
    RenderWindow(0);
  }
  
  if (mmio.dispcnt.enable[DisplayControl::ENABLE_WIN1]) {
    RenderWindow(1);
  }
  
  switch (mmio.dispcnt.mode) {
    case 0: {
      for (int i = 0; i < 4; i++) {
        if (mmio.dispcnt.enable[i]) {
          RenderLayerText(i);
        }
      }
      if (mmio.dispcnt.enable[DisplayControl::ENABLE_OBJ]) {
        RenderLayerOAM();
      }
      ComposeScanline(0, 3);
      break;
    }
  }
  
//  if (mmio.dispcnt.forced_blank) {
//    for (int x = 0; x < 240; x++)
//      line[x] = ConvertColor(0x7FFF);
//  } else {
//    std::uint16_t backdrop = ReadPalette(0, 0);
//    std::uint32_t fill = backdrop * 0x00010001;
//
//    /* Reset scanline buffers. */
//    for (int i = 0; i < 240; i++) {
//      ((std::uint32_t*)pixel)[i] = fill;
//    }
//    for (int i = 0; i < 60; i++) {
//      ((std::uint32_t*)obj_attr)[i] = 0;
//    }
//    for (int i = 0; i < 120; i++) {
//      ((std::uint32_t*)priority)[i] = 0x04040404;
//      ((std::uint32_t*)layer)[i] = 0x05050505;
//    }
//    
//    /* TODO: how does HW behave when we select mode 6 or 7? */
//    switch (mmio.dispcnt.mode) {
//      case 0: {
//        for (int i = 3; i >= 0; i--) {
//          if (mmio.dispcnt.enable[i])
//            RenderLayerText(i);
//        }
//        if (mmio.dispcnt.enable[4])
//          RenderLayerOAM();
//        break;
//      }
//      case 1:
//        if (mmio.dispcnt.enable[0])
//          RenderLayerText(0);
//        if (mmio.dispcnt.enable[1])
//          RenderLayerText(1);
//        if (mmio.dispcnt.enable[2])
//          RenderLayerAffine(0);
//        if (mmio.dispcnt.enable[4])
//          RenderLayerOAM();
//        break;
//      case 2:
//        if (mmio.dispcnt.enable[2])
//          RenderLayerAffine(0);
//        if (mmio.dispcnt.enable[3])
//          RenderLayerAffine(1);
//        if (mmio.dispcnt.enable[4])
//          RenderLayerOAM();
//        break;
//      case 3:
//        break;
//      case 4: {
//        int frame = mmio.dispcnt.frame * 0xA000;
//        int offset = frame + vcount * 240;
//        for (int x = 0; x < 240; x++) {
//          DrawPixel(x, 2, mmio.bgcnt[2].priority, ReadPalette(0, cpu->memory.vram[offset + x]));
//        }
//        break;
//      }
//      case 5:
//        break;
//    }
//
//    auto& bldcnt = mmio.bldcnt;
//        
//    for (int x = 0; x < 240; x++) {
//      auto sfx = bldcnt.sfx;
//      int layer1 = layer[0][x];
//      int layer2 = layer[1][x];
//
//      bool is_alpha_obj = (layer[0][x] == 4) && (obj_attr[x] & OBJ_IS_ALPHA);         
//      bool is_sfx_1 = bldcnt.targets[0][layer1] || is_alpha_obj;
//      bool is_sfx_2 = bldcnt.targets[1][layer2];
//
//      if (is_alpha_obj && is_sfx_2) {
//        sfx = BlendControl::Effect::SFX_BLEND;
//      }
//
//      if (sfx != BlendControl::Effect::SFX_NONE && is_sfx_1 &&
//          (is_sfx_2 || sfx != BlendControl::Effect::SFX_BLEND)) {
//        Blend(pixel[0][x], pixel[1][x], sfx);
//      }
//    }
//
//    for (int x = 0; x < 240; x++) {
//      line[x] = ConvertColor(pixel[0][x]);
//    }
//  }
}
