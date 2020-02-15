/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cstring>

#include "ppu.hpp"
#include "../cpu.hpp"

using namespace GameBoyAdvance;

constexpr std::uint16_t PPU::s_color_transparent;
constexpr int PPU::s_wait_cycles[4];

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
    mmio.bgpa[i] = 0x100;
    mmio.bgpb[i] = 0;
    mmio.bgpc[i] = 0;
    mmio.bgpd[i] = 0x100;
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
//  
  event.countdown = 0;
  SetNextEvent(Phase::SCANLINE);
}

void PPU::Tick() {
  switch (phase) {
    case Phase::SCANLINE: {
      OnScanlineComplete();
      break;
    }
    case Phase::HBLANK: {
      OnHblankComplete();
      break;
    }
    case Phase::VBLANK_SCANLINE: {
      OnVblankScanlineComplete();
      break;
    }
    case Phase::VBLANK_HBLANK: {
      OnVblankHblankComplete();
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
  
  SetNextEvent(Phase::HBLANK);
  cpu->dma.Request(DMA::Occasion::HBlank);
  dispstat.hblank_flag = 1;
  
  if (mmio.vcount >= 2) {
    cpu->dma.Request(DMA::Occasion::Video);
  }

  if (dispstat.hblank_irq_enable) {
    cpu->mmio.irq_if |= CPU::INT_HBLANK;
  }
}

void PPU::OnHblankComplete() {
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
    
    /* Enter vertical blanking mode */
    SetNextEvent(Phase::VBLANK_SCANLINE);
    cpu->dma.Request(DMA::Occasion::VBlank);
    dispstat.vblank_flag = 1;
    
    if (dispstat.vblank_irq_enable) {
      cpu->mmio.irq_if |= CPU::INT_VBLANK;
    }
    
    /* Reset vertical mosaic counters */
    mosaic.bg._counter_y = 0;
    mosaic.obj._counter_y = 0;
    
    /* Reload internal affine registers */
    bgx[0]._current = bgx[0].initial;
    bgy[0]._current = bgy[0].initial;
    bgx[1]._current = bgx[1].initial;
    bgy[1]._current = bgy[1].initial;
  } else {
    /* Advance vertical background mosaic counter */
    if (++mosaic.bg._counter_y == mosaic.bg.size_y) {
      mosaic.bg._counter_y = 0;
    }
    
    /* Advance vertical OBJ mosaic counter */
    if (++mosaic.obj._counter_y == mosaic.obj.size_y) {
      mosaic.obj._counter_y = 0;
    }
    
    /* Vertical mosaic for affine-transformed layers. */
    for (int i = 0; i < 2; i++) {
      /* TODO: I don't know if affine MOSAIC is actually implemented like this. */
      if (mmio.bgcnt[2 + i].mosaic_enable) {
        if (mosaic.bg._counter_y == 0) {
          bgx[i]._current += mosaic.bg.size_y * mmio.bgpb[i];
          bgy[i]._current += mosaic.bg.size_y * mmio.bgpd[i];
        }
      } else {
        bgx[i]._current += mmio.bgpb[i];
        bgy[i]._current += mmio.bgpd[i];
      }
    }
    
    /* Continue to render the next scanline */
    SetNextEvent(Phase::SCANLINE);
    RenderScanline();
  }
}

void PPU::OnVblankScanlineComplete() {
  auto& dispstat = mmio.dispstat;
  
  /* NOTE: HBlank during VBlank does not appear to trigger HBlank DMA.
   * Contrary to GBATEK, HBlank IRQs however appear to happen.
   */
  SetNextEvent(Phase::VBLANK_HBLANK);
  dispstat.hblank_flag = 1;
  
  if (mmio.vcount < 162) {
    cpu->dma.Request(DMA::Occasion::Video);
  }
  
  if (dispstat.hblank_irq_enable) {
    cpu->mmio.irq_if |= CPU::INT_HBLANK;
  }
}

void PPU::OnVblankHblankComplete() {
  auto& vcount = mmio.vcount;
  auto& dispstat = mmio.dispstat;
  
  dispstat.hblank_flag = 0;
    
  if (vcount == 227) {
    /* Update vertical counter. */
    vcount = 0;
    dispstat.vcount_flag = dispstat.vcount_setting == 0;

    /* Leave vertical blanking mode, render first scanline. */
    SetNextEvent(Phase::SCANLINE);
    RenderScanline();
  } else {
    SetNextEvent(Phase::VBLANK_SCANLINE);
    if (vcount == 226) {
      dispstat.vblank_flag = 0;
    }

    /* Update vertical counter. */
    dispstat.vcount_flag = ++vcount == dispstat.vcount_setting;
  }

  if (dispstat.vcount_flag && dispstat.vcount_irq_enable) {
    cpu->mmio.irq_if |= CPU::INT_VCOUNT;
  }
}