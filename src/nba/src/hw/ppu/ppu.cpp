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

PPU::~PPU() {
  StopRenderThread();
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
    window_scanline_enable[i] = false;
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

  SetupRenderThread();
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

void PPU::OnScanlineComplete(int cycles_late) {
  auto& bgx = mmio.bgx;
  auto& bgy = mmio.bgy;
  auto& bgpb = mmio.bgpb;
  auto& bgpd = mmio.bgpd;
  auto& mosaic = mmio.mosaic;

  scheduler.Add(226 - cycles_late, this, &PPU::OnHblankComplete);

  mmio.dispstat.hblank_flag = 1;

  if (mmio.dispstat.hblank_irq_enable) {
    irq.Raise(IRQ::Source::HBlank);
  }

  dma.Request(DMA::Occasion::HBlank);
  
  if (mmio.vcount >= 2) {
    dma.Request(DMA::Occasion::Video);
  }

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

  /* TODO: it appears that this should really happen ~36 cycles into H-draw.
   * But right now if we do that it breaks at least Pinball Tycoon.
   */
  LatchEnabledBGs();
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

  // if (dispcnt.enable[ENABLE_WIN0]) {
  //   RenderWindow(0);
  // }

  // if (dispcnt.enable[ENABLE_WIN1]) {
  //   RenderWindow(1);
  // }

  LatchBGXYWrites();

  if (vcount == 160) {
    // config->video_dev->Draw(output);
    SubmitScanline();

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
    bgx[0]._current = bgx[0].initial;
    bgy[0]._current = bgy[0].initial;
    bgx[1]._current = bgx[1].initial;
    bgy[1]._current = bgy[1].initial;
  } else {
    scheduler.Add(1006 - cycles_late, this, &PPU::OnScanlineComplete);
    // RenderScanline();
    SubmitScanline();
    // // Render OBJs for the next scanline.
    // if (mmio.dispcnt.enable[ENABLE_OBJ]) {
    //   RenderLayerOAM(mmio.dispcnt.mode >= 3, mmio.vcount + 1);
    // }
  }
}

void PPU::OnVblankScanlineComplete(int cycles_late) {
  auto& dispstat = mmio.dispstat;

  scheduler.Add(226 - cycles_late, this, &PPU::OnVblankHblankComplete);

  dispstat.hblank_flag = 1;

  if (mmio.vcount < 162) {
    dma.Request(DMA::Occasion::Video);
  } else if (mmio.vcount == 162) {
    dma.StopVideoXferDMA();
  }

  if (dispstat.hblank_irq_enable) {
    irq.Raise(IRQ::Source::HBlank);
  }

  if (mmio.vcount >= 225) {
    /* TODO: it appears that this should really happen ~36 cycles into H-draw.
     * But right now if we do that it breaks at least Pinball Tycoon.
     */
    LatchEnabledBGs();

    if (mmio.vcount == 227) {
      // Advance vertical OBJ mosaic counter
      if (++mmio.mosaic.obj._counter_y == mmio.mosaic.obj.size_y) {
        mmio.mosaic.obj._counter_y = 0;
      }
    }
  }
}

void PPU::OnVblankHblankComplete(int cycles_late) {
  auto& vcount = mmio.vcount;
  auto& dispstat = mmio.dispstat;

  dispstat.hblank_flag = 0;

  if (vcount == 227) {
    scheduler.Add(1006 - cycles_late, this, &PPU::OnScanlineComplete);
    vcount = 0;

    // Wait for the renderer to complete, then draw.
    while (render_thread_vcount <= render_thread_vcount_max) ;
    config->video_dev->Draw(output[frame]);
    frame ^= 1;
  } else {
    scheduler.Add(1006 - cycles_late, this, &PPU::OnVblankScanlineComplete);
    if (++vcount == 227) {
      dispstat.vblank_flag = 0;
      // Render OBJs for the next scanline
      // if (mmio.dispcnt.enable[ENABLE_OBJ]) {
      //   RenderLayerOAM(mmio.dispcnt.mode >= 3, 0);
      // }
    }
  }

  // if (mmio.dispcnt.enable[ENABLE_WIN0]) {
  //   RenderWindow(0);
  // }

  // if (mmio.dispcnt.enable[ENABLE_WIN1]) {
  //   RenderWindow(1);
  // }

  SubmitScanline();

  CheckVerticalCounterIRQ();
}

void PPU::SetupRenderThread() {
  StopRenderThread();

  render_thread_vcount = 0;
  render_thread_vcount_max = -1;
  render_thread_running = true;

  render_thread = std::thread([this]() {
    while (render_thread_running) {
      while (render_thread_vcount <= render_thread_vcount_max) {
        // TODO: this might be racy with SubmitScanline() resetting render_thread_vcount.
        int vcount = render_thread_vcount;
        auto& mmio = mmio_copy[vcount];

        if (mmio.dispcnt.enable[ENABLE_WIN0]) {
          RenderWindow(vcount, 0);
        }

        if (mmio.dispcnt.enable[ENABLE_WIN1]) {
          RenderWindow(vcount, 1);
        }

        if (vcount < 160) {
          RenderScanline(vcount);
        
          // Render OBJs for the next scanline
          if (mmio.dispcnt.enable[ENABLE_OBJ]) {
            RenderLayerOAM(mmio.dispcnt.mode >= 3, vcount, vcount + 1);
          }
        } else if (vcount == 227) {
          // Render OBJs for the next scanline
          if (mmio.dispcnt.enable[ENABLE_OBJ]) {
            RenderLayerOAM(mmio.dispcnt.mode >= 3, 227, 0);
          }
        }

        render_thread_vcount++;
      }
    }
  });
}

void PPU::StopRenderThread() {
  if (!render_thread_running) {
    return;
  }

  render_thread_running = false;
  render_thread.join();
}

void PPU::SubmitScanline() {
  auto vcount = mmio.vcount;

  if (vcount < 160 || vcount == 227) {
    mmio_copy[vcount] = mmio;
  } else {
    mmio_copy[vcount].dispcnt = mmio.dispcnt;
    mmio_copy[vcount].winh[0] = mmio.winh[0];
    mmio_copy[vcount].winh[1] = mmio.winh[1];
    mmio_copy[vcount].winv[0] = mmio.winv[0];
    mmio_copy[vcount].winv[1] = mmio.winv[1];
  }

  render_thread_vcount_max = vcount;

  if (vcount == 0) {
    // Make a snapshot of PRAM, OAM and VRAM at the start of the frame.
    std::memcpy(pram_draw, pram, sizeof(pram));
    std::memcpy(oam_draw,  oam,  sizeof(oam ));

    int vram_dirty_size = vram_dirty_range_hi - vram_dirty_range_lo;
    if (vram_dirty_size > 0) {
      std::memcpy(&vram_draw[vram_dirty_range_lo], &vram[vram_dirty_range_lo], vram_dirty_size);
      vram_dirty_range_lo = 0x18000;
      vram_dirty_range_hi = 0;
    }

    render_thread_vcount = 0;
  }
}

} // namespace nba::core
