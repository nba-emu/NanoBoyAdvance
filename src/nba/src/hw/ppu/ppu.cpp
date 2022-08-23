/*
 * Copyright (C) 2022 fleroviux
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

  for (int i = 0; i < 4; i++) {
    mmio.enable_bg[0][i] = false;
    mmio.enable_bg[1][i] = false;

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
    window_flag_h[i] = false;
    window_flag_v[i] = false;
  }

  mmio.winin.Reset();
  mmio.winout.Reset();
  mmio.mosaic.Reset();

  mmio.eva = 0;
  mmio.evb = 0;
  mmio.evy = 0;
  mmio.bldcnt.Reset();

  // VCOUNT=225 DISPSTAT=3 was measured after reset on a 3DS in GBA mode (thanks Lady Starbreeze).
  mmio.vcount = 225;
  mmio.dispstat.vblank_flag = true;
  mmio.dispstat.hblank_flag = true;
  scheduler.Add(226, this, &PPU::OnVblankHblankComplete);

  frame = 0;
  dma3_video_transfer_running = false;

  pram_access = false;
  vram_bg_access = false;
}

void PPU::LatchEnabledBGs() {
  for (int i = 0; i < 4; i++) {
    mmio.enable_bg[0][i] = mmio.enable_bg[1][i];
  }

  for (int i = 0; i < 4; i++) {
    mmio.enable_bg[1][i] = mmio.dispcnt.enable[i];
  }
}

void PPU::LatchBGXYWrites() {
  // TODO: should BGY be latched when BGX was written and vice versa?
  for (int i = 0; i < 2; i++) {
    auto& bgx = mmio.bgx[i];
    auto& bgy = mmio.bgy[i];

    if (bgx.written) {
      bgx._current = bgx.initial;
      bgx.written = false;
    }

    if (bgy.written) {
      bgy._current = bgy.initial;
      bgy.written = false;
    }
  }
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

void PPU::OnScanlineComplete(int cycles_late) {
  auto& bgx = mmio.bgx;
  auto& bgy = mmio.bgy;
  auto& bgpb = mmio.bgpb;
  auto& bgpd = mmio.bgpd;
  auto& mosaic = mmio.mosaic;

  mmio.dispstat.hblank_flag = 1;

  dma.Request(DMA::Occasion::HBlank);
  
  // Advance vertical background mosaic counter
  if (++mosaic.bg._counter_y == mosaic.bg.size_y) {
    mosaic.bg._counter_y = 0;
  }

  // Advance vertical OBJ mosaic counter
  if (++mmio.mosaic.obj._counter_y == mmio.mosaic.obj.size_y) {
    mmio.mosaic.obj._counter_y = 0;
  }

  /* Mode 0 doesn't have any affine backgrounds,
   * in that case the internal X/Y registers will never be updated.
   */
  if (mmio.dispcnt.mode != 0) {
    // Advance internal affine registers and apply vertical mosaic if applicable.
    for (int i = 0; i < 2; i++) {
      /* Do not update internal X/Y unless the latched BG enable bit is set.
       * This behavior was confirmed on real hardware.
       */
      auto bg_id = 2 + i;

      if (mmio.enable_bg[0][bg_id]) {
        if (mmio.bgcnt[bg_id].mosaic_enable) {
          if (mosaic.bg._counter_y == 0) {
            bgx[i]._current += mosaic.bg.size_y * bgpb[i];
            bgy[i]._current += mosaic.bg.size_y * bgpd[i];
          }
        } else {
          bgx[i]._current += bgpb[i];
          bgy[i]._current += bgpd[i];
        }
      }
    }
  }

  SyncLineRender();
  // Render OBJs for the next scanline.
  if (mmio.dispcnt.enable[ENABLE_OBJ]) {
    RenderLayerOAM(mmio.dispcnt.mode >= 3, mmio.vcount + 1);
  }

  /* TODO: it appears that this should really happen ~36 cycles into H-draw.
   * But right now if we do that it breaks at least Pinball Tycoon.
   */
  LatchEnabledBGs();

  scheduler.Add(4 - cycles_late, this, &PPU::OnHblankIRQTest);
}

void PPU::OnHblankIRQTest(int cycles_late) {
  // TODO: confirm that the enable-bit is checked at 1010 and not 1006 cycles.
  if (mmio.dispstat.hblank_irq_enable) {
    irq.Raise(IRQ::Source::HBlank);
  }

  scheduler.Add(222 - cycles_late, this, &PPU::OnHblankComplete);
}

void PPU::OnHblankComplete(int cycles_late) {
  auto& dispcnt = mmio.dispcnt;
  auto& dispstat = mmio.dispstat;
  auto& vcount = mmio.vcount;
  auto& bgx = mmio.bgx;
  auto& bgy = mmio.bgy;
  auto& mosaic = mmio.mosaic;

  dispstat.hblank_flag = 0;
  vcount++;

  CheckVerticalCounterIRQ();
  LatchBGXYWrites();
  UpdateVideoTransferDMA();
  UpdateWindows();

  if (vcount == 160) {
    scheduler.Add(1006 - cycles_late, this, &PPU::OnVblankScanlineComplete);
    dma.Request(DMA::Occasion::VBlank);
    dispstat.vblank_flag = 1;

    if (dispstat.vblank_irq_enable) {
      irq.Raise(IRQ::Source::VBlank);
    }

    // Reset vertical mosaic counters
    mosaic.bg._counter_y = 0;
    mosaic.obj._counter_y = 0;

    // Reload internal affine registers
    for (int i = 0; i < 2; i++) {
      bgx[i]._current = bgx[i].initial;
      bgy[i]._current = bgy[i].initial;
    }
  } else {
    scheduler.Add(1006 - cycles_late, this, &PPU::OnScanlineComplete);
    InitLineRender();
  }
}

void PPU::OnVblankScanlineComplete(int cycles_late) {
  auto& dispstat = mmio.dispstat;

  dispstat.hblank_flag = 1;

  if (mmio.vcount >= 225) {
    /* TODO: it appears that this should really happen ~36 cycles into H-draw.
     * But right now if we do that it breaks at least Pinball Tycoon.
     */
    LatchEnabledBGs();

    if (mmio.vcount == 227) {
      // Render OBJs for the next scanline.
      if (mmio.dispcnt.enable[ENABLE_OBJ]) {
        RenderLayerOAM(mmio.dispcnt.mode >= 3, 0);
      }

      // Advance vertical OBJ mosaic counter
      if (++mmio.mosaic.obj._counter_y == mmio.mosaic.obj.size_y) {
        mmio.mosaic.obj._counter_y = 0;
      }
    }
  }

  scheduler.Add(4 - cycles_late, this, &PPU::OnVblankHblankIRQTest);
}

void PPU::OnVblankHblankIRQTest(int cycles_late) {
  // TODO: confirm that the enable-bit is checked at 1010 and not 1006 cycles.
  if (mmio.dispstat.hblank_irq_enable) {
    irq.Raise(IRQ::Source::HBlank);
  }

  scheduler.Add(222 - cycles_late, this, &PPU::OnVblankHblankComplete);
}

void PPU::OnVblankHblankComplete(int cycles_late) {
  auto& vcount = mmio.vcount;
  auto& dispstat = mmio.dispstat;

  dispstat.hblank_flag = 0;

  if (vcount == 162) {
    /**
     * TODO:
     *  - figure out when precisely DMA3CNT is latched
     *  - figure out what bits of DMA3CNT are checked
     */
    dma3_video_transfer_running = dma.HasVideoTransferDMA();
  }

  if (vcount == 227) {
    scheduler.Add(1006 - cycles_late, this, &PPU::OnScanlineComplete);
    vcount = 0;

    // Wait for the renderer to complete, then draw.
    // while (render_thread_vcount <= render_thread_vcount_max) ;
    config->video_dev->Draw(output[frame]);
    frame ^= 1;

    InitLineRender();
  } else {
    scheduler.Add(1006 - cycles_late, this, &PPU::OnVblankScanlineComplete);
    if (++vcount == 227) {
      dispstat.vblank_flag = 0;
    }
  }

  LatchBGXYWrites();
  CheckVerticalCounterIRQ();
  UpdateVideoTransferDMA();
  UpdateWindows();
}

void PPU::InitLineRender() {
  // Minimum and maximum available BGs in each BG mode:
  const int MIN_BG[8] { 0, 0, 2, 2, 2, 2, -1, -1 };
  const int MAX_BG[8] { 3, 2, 3, 2, 2, 2, -1, -1 };

  int mode = mmio.dispcnt.mode;

  dispcnt_mode = mode;
  last_sync_point = scheduler.GetTimestampNow();

  int min_bg = MIN_BG[mode];
  int max_bg = MAX_BG[mode];

  for (int id = 0; id < 4; id++) {
    bg[id].engaged = false;
  }

  compose.engaged = false;

  if (mmio.dispcnt.forced_blank) {
    u32* buffer = &output[frame][mmio.vcount * 240];

    // TODO: figure out how the 'forced blank' bit works on hardware
    for (int x = 0; x < 240; x++) {
      buffer[x] = 0xFFF8F8F8;
    }
  } else {
    for (int id = min_bg; id <= max_bg; id++) {
      if (mmio.enable_bg[0][id]) {
        InitBG(id);
      }
    }

    InitCompose();
  }
}

void PPU::SyncLineRender() {
  u64 sync_point = scheduler.GetTimestampNow();

  int cycles = (int)(sync_point - last_sync_point);

  /**
   * Do not sync multiple times on the same cycle.
   * This can happen, when this method is called from an event,
   * in the same cycle as a VRAM/PRAM/OAM/IO access.
   *
   * It is necessary to handle this, so that VRAM access stalls
   * are calculated correctly in that scenario.
   */
  if (cycles == 0) {
    return;
  }

  pram_access = false;
  vram_bg_access = false;

  for (int id = 0; id < 4; id++) {
    if (bg[id].engaged) {
      SyncBG(id, cycles);
    }
  }

  if (compose.engaged) {
    SyncCompose(cycles);
  }
  
  last_sync_point = sync_point;
}

void PPU::UpdateWindows() {
  int vcount = mmio.vcount;

  for (int i = 0; i < 2; i++) {
    if (vcount == mmio.winv[i].min) window_flag_v[i] = true;
    if (vcount == mmio.winv[i].max) window_flag_v[i] = false;
  }
}

} // namespace nba::core
