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
        
    mmio.bgx[0]._current = mmio.bgx[0].initial;
    mmio.bgy[0]._current = mmio.bgy[0].initial;
    mmio.bgx[1]._current = mmio.bgx[1].initial;
    mmio.bgy[1]._current = mmio.bgy[1].initial;
  } else {                
    SetNextEvent(Phase::SCANLINE);
    RenderScanline();
    
    /* TODO: not sure if this should be done before or after RenderScanline() */
    mmio.bgx[0]._current += mmio.bgpb[0];
    mmio.bgy[0]._current += mmio.bgpd[0];
    mmio.bgx[1]._current += mmio.bgpb[1];
    mmio.bgy[1]._current += mmio.bgpd[1];
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
