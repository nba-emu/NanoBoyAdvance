/*
 * Copyright (C) 2024 fleroviux
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
  scheduler.Register(Scheduler::EventClass::PPU_hdraw_vdraw, this, &PPU::BeginHDrawVDraw);
  scheduler.Register(Scheduler::EventClass::PPU_hblank_vdraw, this, &PPU::BeginHBlankVDraw);
  scheduler.Register(Scheduler::EventClass::PPU_hdraw_vblank, this, &PPU::BeginHDrawVBlank);
  scheduler.Register(Scheduler::EventClass::PPU_hblank_vblank, this, &PPU::BeginHBlankVBlank);
  scheduler.Register(Scheduler::EventClass::PPU_begin_sprite_fetch, this, &PPU::BeginSpriteDrawing);

  scheduler.Register(Scheduler::EventClass::PPU_update_vcount_flag, this, &PPU::UpdateVerticalCounterFlag);
  scheduler.Register(Scheduler::EventClass::PPU_video_dma, this, &PPU::RequestVideoDMA);
  scheduler.Register(Scheduler::EventClass::PPU_latch_dispcnt, this, &PPU::LatchDISPCNT);
  scheduler.Register(Scheduler::EventClass::PPU_hblank_irq, this, &PPU::RequestHblankIRQ);
  scheduler.Register(Scheduler::EventClass::PPU_vblank_irq, this, &PPU::RequestVblankIRQ);
  scheduler.Register(Scheduler::EventClass::PPU_vcount_irq, this, &PPU::RequestVcountIRQ);

  mmio.dispcnt.ppu = this;
  mmio.dispstat.ppu = this;
  Reset();
}

void PPU::Reset() {
  std::memset(pram, 0, 0x00400);
  std::memset(oam,  0, 0x00400);
  std::memset(vram, 0, 0x18000);

  vram_bg_latch = 0U;

  mmio.dispcnt.Reset();
  mmio.dispstat.Reset();

  mmio.greenswap = 0U;

  for(int i = 0; i < 4; i++) {
    mmio.bgcnt[i].Reset();
    mmio.bghofs[i] = 0;
    mmio.bgvofs[i] = 0;
  }

  for(int i = 0; i < 2; i++) {
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
  sprite.buffer_rd = sprite.buffer[0];
  sprite.buffer_wr = sprite.buffer[1];
  window = {};
  merge = {};

  frame = 0;
  dma3_video_transfer_running = false;
}

void PPU::BeginHDrawVDraw() {
  auto& dispstat = mmio.dispstat;
  auto& vcount = mmio.vcount;

  DrawBackground();
  DrawWindow();
  DrawMerge();

  scheduler.Add(1, Scheduler::EventClass::PPU_update_vcount_flag);
  scheduler.Add(40, Scheduler::EventClass::PPU_latch_dispcnt);

  dispstat.hblank_flag = 0;
  vcount++;

  UpdateVideoTransferDMA();

  if(vcount == 160) {
    scheduler.Add(1007, Scheduler::EventClass::PPU_hblank_vblank);
    RequestVblankDMA();
    dispstat.vblank_flag = 1;

    if(dispstat.vblank_irq_enable) {
      scheduler.Add(1, Scheduler::EventClass::PPU_vblank_irq);
    }
  } else {
    InitBackground();
    InitMerge();

    scheduler.Add(1007, Scheduler::EventClass::PPU_hblank_vdraw);
  }

  InitWindow();
}

void PPU::BeginHBlankVDraw() {
  mmio.dispstat.hblank_flag = 1;

  RequestHblankDMA();

  if(mmio.dispstat.hblank_irq_enable) {
    scheduler.Add(1, Scheduler::EventClass::PPU_hblank_irq);
  }

  scheduler.Add(225, Scheduler::EventClass::PPU_hdraw_vdraw);
}

void PPU::BeginHDrawVBlank() {
  auto& vcount = mmio.vcount;
  auto& dispstat = mmio.dispstat;

  DrawWindow();

  scheduler.Add(1, Scheduler::EventClass::PPU_update_vcount_flag);

  dispstat.hblank_flag = 0;

  if(vcount == 162) {
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

  if(vcount == 227) {
    scheduler.Add(1007, Scheduler::EventClass::PPU_hblank_vdraw);
    vcount = 0;

    config->video_dev->Draw(output[frame]);
    frame ^= 1;

    InitBackground();
    InitMerge();
  } else {
    scheduler.Add(1007, Scheduler::EventClass::PPU_hblank_vblank);
    
    if(++vcount == 227) {
      dispstat.vblank_flag = 0;
    }
  }

  UpdateVideoTransferDMA();

  InitWindow();
}

void PPU::BeginHBlankVBlank() {
  auto& dispstat = mmio.dispstat;

  dispstat.hblank_flag = 1;

  if(mmio.dispstat.hblank_irq_enable) {
    scheduler.Add(1, Scheduler::EventClass::PPU_hblank_irq);
  }

  scheduler.Add(225, Scheduler::EventClass::PPU_hdraw_vblank);
}

void PPU::BeginSpriteDrawing() {
  const uint vcount = mmio.vcount;

  if(vcount < 160U) {
    DrawSprite();
  }

  if(vcount == 227U || vcount < 160U) {
    std::swap(sprite.buffer_rd, sprite.buffer_wr);

    if(vcount != 159U) {
      InitSprite();
    }
  }

  scheduler.Add(1232, Scheduler::EventClass::PPU_begin_sprite_fetch);
}

void PPU::UpdateVerticalCounterFlag() {
  auto& dispstat = mmio.dispstat;
  auto vcount_flag_new = dispstat.vcount_setting == mmio.vcount;

  if(dispstat.vcount_irq_enable && !dispstat.vcount_flag && vcount_flag_new) {
    // @todo: why is it necessary to set the event priority here?
    scheduler.Add(1, Scheduler::EventClass::PPU_vcount_irq, 1);
  }
  
  dispstat.vcount_flag = vcount_flag_new;
}

void PPU::UpdateVideoTransferDMA() {
  int vcount = mmio.vcount;

  if(dma3_video_transfer_running) {
    if(vcount == 162) {
      dma.StopVideoTransferDMA();
    } else if(vcount >= 2 && vcount < 162) {
      scheduler.Add(3, Scheduler::EventClass::PPU_video_dma);
    }
  }
}

void PPU::LatchDISPCNT() {
  mmio.dispcnt_latch[0] = mmio.dispcnt_latch[1];
  mmio.dispcnt_latch[1] = mmio.dispcnt_latch[2];
  mmio.dispcnt_latch[2] = mmio.dispcnt.hword;
}

} // namespace nba::core
