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
  
  mmio.winh[0].Reset();
  mmio.winh[1].Reset();
  mmio.winv[0].Reset();
  mmio.winv[1].Reset();
  mmio.winin.Reset();
  mmio.winout.Reset();
  
  mmio.mosaic.Reset();
  
  mmio.eva = 0;
  mmio.evb = 0;
  mmio.evy = 0;
  mmio.bldcnt.Reset();
  
//  for (int x = 0; x < 240; x++) {
//    for (int bg = 0; bg < 4; bg++) {
//      buffer_bg[bg][x] = s_color_transparent;
//    }
//    buffer_obj[x].priority = 4;
//    buffer_obj[x].color = s_color_transparent;
//    buffer_obj[x].alpha = 0;
//    buffer_obj[x].window = 0;
//  }
//  
//  line_contains_alpha_obj = false;
  
  event.countdown = 0;
  SetNextEvent(Phase::SCANLINE);
}

void PPU::Tick() {
  auto& vcount = mmio.vcount;
  auto& dispstat = mmio.dispstat;

  switch (phase) {
    case Phase::SCANLINE: {
      OnScanlineComplete();
      break;
    }
    case Phase::HBLANK: {
      OnHBlankComplete();
      break;
    }
    case Phase::VBLANK: {
      OnVBlankLineComplete();
      break;
    }
  }
}

void PPU::SetNextEvent(Phase phase) {
  this->phase = phase;
  event.countdown += s_wait_cycles[static_cast<int>(phase)];
}

void PPU::OnScanlineComplete() {
  auto& dispstat = mmio.dispstat;
  
  cpu->dma.Request(DMA::Occasion::HBlank);
  SetNextEvent(Phase::HBLANK);
  dispstat.hblank_flag = 1;

  if (dispstat.hblank_irq_enable) {
    cpu->mmio.irq_if |= CPU::INT_HBLANK;
  }
}

void PPU::OnHBlankComplete() {
  auto& vcount = mmio.vcount;
  auto& dispstat = mmio.dispstat;
  auto& mosaic = mmio.mosaic;
  auto& bgx = mmio.bgx;
  auto& bgy = mmio.bgy;
  
  dispstat.hblank_flag = 0;
  dispstat.vcount_flag = ++vcount == dispstat.vcount_setting;

  if (dispstat.vcount_flag && dispstat.vcount_irq_enable) {
    cpu->mmio.irq_if |= CPU::INT_VCOUNT;
  }

  if (vcount == 160) {
    cpu->config->video_dev->Draw(output);
      
    cpu->dma.Request(DMA::Occasion::VBlank);
    dispstat.vblank_flag = 1;
    SetNextEvent(Phase::VBLANK);

    if (dispstat.vblank_irq_enable) {
      cpu->mmio.irq_if |= CPU::INT_VBLANK;
    }
    
    mosaic.bg._counter_y = 0;
    mosaic.obj._counter_y = 0;
    
    bgx[0]._current = bgx[0].initial;
    bgy[0]._current = bgy[0].initial;
    bgx[1]._current = bgx[1].initial;
    bgy[1]._current = bgy[1].initial;
  } else {    
    if (++mosaic.bg._counter_y == mosaic.bg.size_y) {
      mosaic.bg._counter_y = 0;
    }
    
    if (++mosaic.obj._counter_y == mosaic.obj.size_y) {
      mosaic.obj._counter_y = 0;
    }
    
    /* TODO: I don't know if affine MOSAIC is actually implemented like this. */
    if (mmio.bgcnt[2].mosaic_enable) {
      if (mosaic.bg._counter_y == 0) {
        bgx[0]._current += mosaic.bg.size_y * mmio.bgpb[0];
        bgy[0]._current += mosaic.bg.size_y * mmio.bgpd[0];
      }
    } else {
      bgx[0]._current += mmio.bgpb[0];
      bgy[0]._current += mmio.bgpd[0];
    }
    
    if (mmio.bgcnt[3].mosaic_enable) {
      if (mosaic.bg._counter_y == 0) {
        bgx[1]._current += mosaic.bg.size_y * mmio.bgpb[1];
        bgy[1]._current += mosaic.bg.size_y * mmio.bgpd[1];
      }
    } else {
      bgx[1]._current += mmio.bgpb[1];
      bgy[1]._current += mmio.bgpd[1];
    }
    
    SetNextEvent(Phase::SCANLINE);
    RenderScanline();
  }
}

void PPU::OnVBlankLineComplete() {
  auto& vcount = mmio.vcount;
  auto& dispstat = mmio.dispstat;
  
  if (vcount == 227) {
    SetNextEvent(Phase::SCANLINE);

    /* Update vertical counter. */
    vcount = 0;
    dispstat.vcount_flag = dispstat.vcount_setting == 0;

    RenderScanline();
  } else {
    SetNextEvent(Phase::VBLANK);
    if (vcount == 226)
      dispstat.vblank_flag = 0;
        
    /* Update vertical counter. */
    dispstat.vcount_flag = ++vcount == dispstat.vcount_setting;
  }

  if (dispstat.vcount_flag && dispstat.vcount_irq_enable) {
    cpu->mmio.irq_if |= CPU::INT_VCOUNT;
  }
}
