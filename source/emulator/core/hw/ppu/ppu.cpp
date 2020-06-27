/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cstring>

#include "ppu.hpp"

namespace nba::core {

constexpr std::uint16_t PPU::s_color_transparent;
constexpr int PPU::s_wait_cycles[5];

PPU::PPU(SchedulerNew* scheduler,
         InterruptController* irq_controller,
         DMA* dma,
         std::shared_ptr<Config> config)
  : scheduler(scheduler)
  , irq_controller(irq_controller)
  , dma(dma)
  , config(config)
{
  InitBlendTable();
  Reset();
  mmio.dispstat.ppu = this;
}

void PPU::Reset() {
  std::memset(pram, 0, 0x00400);
  std::memset(oam,  0, 0x00400);
  std::memset(vram, 0, 0x18000);

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

  SetNextEvent(Phase::SCANLINE, 0);
}

void PPU::SetNextEvent(Phase phase, int cycles_late) {
  this->phase = phase;
  scheduler->Add(s_wait_cycles[static_cast<int>(phase)] - cycles_late, event_cb);
}

void PPU::Tick(int cycles_late) {
  // TODO: get rid of the indirection and schedule the appropriate method directly.
  switch (phase) {
    case Phase::SCANLINE:
      OnScanlineComplete(cycles_late);
      break;
    case Phase::HBLANK_SEARCH:
      OnHblankSearchComplete(cycles_late);
      break;
    case Phase::HBLANK:
      OnHblankComplete(cycles_late);
      break;
    case Phase::VBLANK_SCANLINE:
      OnVblankScanlineComplete(cycles_late);
      break;
    case Phase::VBLANK_HBLANK:
      OnVblankHblankComplete(cycles_late);
      break;
  }
}

void PPU::UpdateInternalAffineRegisters() {
  auto& bgx = mmio.bgx;
  auto& bgy = mmio.bgy;
  auto& mosaic = mmio.mosaic;

  if (mmio.vcount == 160 || mmio.dispcnt._mode_is_dirty) {
    mmio.dispcnt._mode_is_dirty = false;

    /* Reload internal affine registers */
    bgx[0]._current = bgx[0].initial;
    bgy[0]._current = bgy[0].initial;
    bgx[1]._current = bgx[1].initial;
    bgy[1]._current = bgy[1].initial;
  } else {
    for (int i = 0; i < 2; i++) {
      if (mmio.bgcnt[2 + i].mosaic_enable) {
        /* Vertical mosaic for affine-transformed layers. */
        if (mosaic.bg._counter_y == 0) {
          bgx[i]._current += mosaic.bg.size_y * mmio.bgpb[i];
          bgy[i]._current += mosaic.bg.size_y * mmio.bgpd[i];
        }
      } else {
        bgx[i]._current += mmio.bgpb[i];
        bgy[i]._current += mmio.bgpd[i];
      }
    }
  }
}

void PPU::CheckVerticalCounterIRQ() {
  auto& dispstat = mmio.dispstat;
  auto vcount_flag_new = dispstat.vcount_setting == mmio.vcount;
  if (dispstat.vcount_irq_enable && !dispstat.vcount_flag && vcount_flag_new) {
    irq_controller->Raise(InterruptSource::VCount);
  }
  dispstat.vcount_flag = vcount_flag_new;
}

void PPU::OnScanlineComplete(int cycles_late) {
  SetNextEvent(Phase::HBLANK_SEARCH, cycles_late);

  if (mmio.dispstat.hblank_irq_enable) {
    irq_controller->Raise(InterruptSource::HBlank);
  }

  RenderScanline();
}

void PPU::OnHblankSearchComplete(int cycles_late) {
  SetNextEvent(Phase::HBLANK, cycles_late);

  dma->Request(DMA::Occasion::HBlank);
  if (mmio.vcount >= 2) {
    dma->Request(DMA::Occasion::Video);
  }
  mmio.dispstat.hblank_flag = 1;
}

void PPU::OnHblankComplete(int cycles_late) {
  auto& vcount = mmio.vcount;
  auto& dispstat = mmio.dispstat;
  auto& mosaic = mmio.mosaic;

  dispstat.hblank_flag = 0;
  vcount++;
  CheckVerticalCounterIRQ();

  if (vcount == 160) {
    config->video_dev->Draw(output);

    SetNextEvent(Phase::VBLANK_SCANLINE, cycles_late);
    dma->Request(DMA::Occasion::VBlank);
    dispstat.vblank_flag = 1;

    if (dispstat.vblank_irq_enable) {
      irq_controller->Raise(InterruptSource::VBlank);
    }

    /* Reset vertical mosaic counters */
    mosaic.bg._counter_y = 0;
    mosaic.obj._counter_y = 0;
  } else {
    /* Advance vertical background mosaic counter */
    if (++mosaic.bg._counter_y == mosaic.bg.size_y) {
      mosaic.bg._counter_y = 0;
    }

    /* Advance vertical OBJ mosaic counter */
    if (++mosaic.obj._counter_y == mosaic.obj.size_y) {
      mosaic.obj._counter_y = 0;
    }

    SetNextEvent(Phase::SCANLINE, cycles_late);
  }

  UpdateInternalAffineRegisters();
}

void PPU::OnVblankScanlineComplete(int cycles_late) {
  auto& dispstat = mmio.dispstat;

  SetNextEvent(Phase::VBLANK_HBLANK, cycles_late);
  dispstat.hblank_flag = 1;

  if (mmio.vcount < 162) {
    dma->Request(DMA::Occasion::Video);
  } else if (mmio.vcount == 162) {
    dma->StopVideoXferDMA();
  }

  if (dispstat.hblank_irq_enable) {
    irq_controller->Raise(InterruptSource::HBlank);
  }
}

void PPU::OnVblankHblankComplete(int cycles_late) {
  auto& vcount = mmio.vcount;
  auto& dispstat = mmio.dispstat;

  dispstat.hblank_flag = 0;

  if (vcount == 227) {
    vcount = 0;
    SetNextEvent(Phase::SCANLINE, cycles_late);
  } else {
    SetNextEvent(Phase::VBLANK_SCANLINE, cycles_late);
    if (vcount == 226) {
      dispstat.vblank_flag = 0;
    }
    vcount++;
  }

  CheckVerticalCounterIRQ();
}

} // namespace nba::core
