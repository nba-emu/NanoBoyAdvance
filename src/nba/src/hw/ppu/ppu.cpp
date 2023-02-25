/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cstring>

#include "hw/ppu/ppu.hpp"

namespace nba::core {

PPU::PPU(
  Scheduler& scheduler,
  IRQ& irq,
  DMA& dma,
  std::shared_ptr<Config> config
)   : scheduler(scheduler)
    , irq(irq)
    , dma(dma)
    , config(config) {
  scheduler.Register(Scheduler::EventClass::PPU_hdraw_vdraw, this, &PPU::OnHblankComplete);
  scheduler.Register(Scheduler::EventClass::PPU_hblank_vdraw, this, &PPU::OnScanlineComplete);
  scheduler.Register(Scheduler::EventClass::PPU_hblank_irq_vdraw, this, &PPU::OnHblankIRQTest);

  scheduler.Register(Scheduler::EventClass::PPU_hdraw_vblank, this, &PPU::OnVblankHblankComplete);
  scheduler.Register(Scheduler::EventClass::PPU_hblank_vblank, this, &PPU::OnVblankScanlineComplete);
  scheduler.Register(Scheduler::EventClass::PPU_hblank_irq_vblank, this, &PPU::OnVblankHblankIRQTest);

  scheduler.Register(Scheduler::EventClass::PPU_begin_sprite_fetch, this, &PPU::StupidSpriteEventHandler);
  scheduler.Register(Scheduler::EventClass::PPU_latch_dispcnt, this, &PPU::LatchDISPCNT);

  mmio.dispcnt.ppu = this;
  mmio.dispstat.ppu = this;
  Reset();
}

void PPU::Reset() {
  std::memset(pram, 0, 0x00400);
  std::memset(oam,  0, 0x00400);
  std::memset(vram, 0, 0x18000);

  mmio.dispcnt.Reset();
  mmio.dispstat.Reset();

  mmio.greenswap = 0U;

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

    mmio.winh[i].Reset();
    mmio.winv[i].Reset();
  }

  mmio.winin.Reset();
  mmio.winout.Reset();
  mmio.mosaic.Reset();

  mmio.eva = 0;
  mmio.evb = 0;
  mmio.evy = 0;
  mmio.bldcnt.Reset();

  for(int i = 0; i < 3; i++) {
    mmio.dispcnt_latch[i] = 0U;
  }

  // VCOUNT=225 DISPSTAT=3 was measured after reset on a 3DS in GBA mode (thanks Lady Starbreeze).
  mmio.vcount = 225;
  mmio.dispstat.vblank_flag = true;
  mmio.dispstat.hblank_flag = true;
  scheduler.Add(226, Scheduler::EventClass::PPU_hdraw_vblank);

  // To keep the state machine simple, we run sprite engine
  // from a separate event loop.
  scheduler.Add(266, Scheduler::EventClass::PPU_begin_sprite_fetch);

  // @todo: initialize window with the appropriate timing.
  bg = {};
  sprite = {};
  window = {};
  merge = {};

  frame = 0;
  dma3_video_transfer_running = false;
}

void PPU::StupidSpriteEventHandler() {
  if(mmio.vcount <= 159) {
    DrawSprite();
  }

  if(mmio.vcount == 227 || mmio.vcount < 159) {
    InitSprite();
  }

  scheduler.Add(1232, Scheduler::EventClass::PPU_begin_sprite_fetch);
}

void PPU::LatchDISPCNT() {
  mmio.dispcnt_latch[0] = mmio.dispcnt_latch[1];
  mmio.dispcnt_latch[1] = mmio.dispcnt_latch[2];
  mmio.dispcnt_latch[2] = mmio.dispcnt.hword;
}

void PPU::CheckVerticalCounterIRQ() {
  auto& dispstat = mmio.dispstat;
  auto vcount_flag_new = dispstat.vcount_setting == mmio.vcount;

  if (dispstat.vcount_irq_enable && !dispstat.vcount_flag && vcount_flag_new) {
    irq.Raise(IRQ::Source::VCount);
  }
  
  dispstat.vcount_flag = vcount_flag_new;
}

void PPU::UpdateVideoTransferDMA() {
  int vcount = mmio.vcount;

  if (dma3_video_transfer_running) {
    if (vcount == 162) {
      dma.StopVideoTransferDMA();
    } else if (vcount >= 2 && vcount < 162) {
      scheduler.Add(9, [this](int late) {
        dma.Request(DMA::Occasion::Video);
      });
    }
  }
}

void PPU::OnScanlineComplete() {
  mmio.dispstat.hblank_flag = 1;

  // TODO: fix these timings properly eventually.
  scheduler.Add(1, [this](int) {
    dma.Request(DMA::Occasion::HBlank);
  });

  scheduler.Add(2, Scheduler::EventClass::PPU_hblank_irq_vdraw);
}

void PPU::OnHblankIRQTest() {
  // TODO: confirm that the enable-bit is checked at 1010 and not 1006 cycles.
  if (mmio.dispstat.hblank_irq_enable) {
    irq.Raise(IRQ::Source::HBlank);
  }

  scheduler.Add(224, Scheduler::EventClass::PPU_hdraw_vdraw);
}

void PPU::OnHblankComplete() {
  auto& dispcnt = mmio.dispcnt;
  auto& dispstat = mmio.dispstat;
  auto& vcount = mmio.vcount;
  auto& bgx = mmio.bgx;
  auto& bgy = mmio.bgy;

  DrawBackground();
  DrawWindow();
  DrawMerge();

  scheduler.Add(40, Scheduler::EventClass::PPU_latch_dispcnt);

  dispstat.hblank_flag = 0;
  vcount++;

  CheckVerticalCounterIRQ();
  UpdateVideoTransferDMA();

  if (vcount == 160) {
    // ScheduleSubmitScanline();

    scheduler.Add(1006, Scheduler::EventClass::PPU_hblank_vblank);
    dma.Request(DMA::Occasion::VBlank);
    dispstat.vblank_flag = 1;

    if (dispstat.vblank_irq_enable) {
      irq.Raise(IRQ::Source::VBlank);
    }

    // Reload internal affine registers
    for (int i = 0; i < 2; i++) {
      bgx[i]._current = bgx[i].initial;
      bgy[i]._current = bgy[i].initial;
    }
  } else {
    InitBackground();
    InitMerge();

    scheduler.Add(1006, Scheduler::EventClass::PPU_hblank_vdraw);
    // ScheduleSubmitScanline();
  }

  InitWindow();
}

void PPU::OnVblankScanlineComplete() {
  auto& dispstat = mmio.dispstat;

  dispstat.hblank_flag = 1;

  scheduler.Add(2, Scheduler::EventClass::PPU_hblank_irq_vblank);
}

void PPU::OnVblankHblankIRQTest() {
  // TODO: confirm that the enable-bit is checked at 1010 and not 1006 cycles.
  if (mmio.dispstat.hblank_irq_enable) {
    irq.Raise(IRQ::Source::HBlank);
  }

  scheduler.Add(224, Scheduler::EventClass::PPU_hdraw_vblank);
}

void PPU::OnVblankHblankComplete() {
  auto& vcount = mmio.vcount;
  auto& dispstat = mmio.dispstat;

  DrawWindow();

  dispstat.hblank_flag = 0;

  if (vcount == 162) {
    /**
     * TODO:
     *  - figure out when precisely DMA3CNT is latched
     *  - figure out what bits of DMA3CNT are checked
     */
    dma3_video_transfer_running = dma.HasVideoTransferDMA();
  }

  if(vcount >= 224) {
    scheduler.Add(40, Scheduler::EventClass::PPU_latch_dispcnt);
  }

  if (vcount == 227) {
    scheduler.Add(1006, Scheduler::EventClass::PPU_hblank_vdraw);
    vcount = 0;

    config->video_dev->Draw(output[frame]);
    frame ^= 1;

    InitBackground();
    InitMerge();
  } else {
    scheduler.Add(1006, Scheduler::EventClass::PPU_hblank_vblank);
    
    if (++vcount == 227) {
      dispstat.vblank_flag = 0;
    }
  }

  CheckVerticalCounterIRQ();
  // ScheduleSubmitScanline();
  UpdateVideoTransferDMA();

  InitWindow();
}

} // namespace nba::core
