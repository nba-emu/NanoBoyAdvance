/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cstring>

#include "ppu.hpp"

namespace nba::core {

void PPU::LoadState(SaveState const& state) {
  auto& ss_ppu = state.ppu;
  auto& mosaic = mmio.mosaic;

  /**
   * We have to restore VCOUNT before DISPSTAT,
   * because otherwise loading DISPSTAT might (incorrectly) trigger a V-count IRQ.
   */
  mmio.vcount = ss_ppu.io.vcount;

  mmio.dispcnt.WriteHalf(ss_ppu.io.dispcnt);
  mmio.greenswap = ss_ppu.io.greenswap;
  mmio.dispstat.WriteHalf(ss_ppu.io.dispstat);

  for(int id = 0; id < 4; id++) {
    mmio.bgcnt[id].WriteHalf(ss_ppu.io.bgcnt[id]);
    mmio.bghofs[id] = ss_ppu.io.bghofs[id];
    mmio.bgvofs[id] = ss_ppu.io.bgvofs[id];
  }

  for(int id = 0; id < 2; id++) {
    mmio.bgpa[id] = ss_ppu.io.bgpa[id];
    mmio.bgpb[id] = ss_ppu.io.bgpb[id];
    mmio.bgpc[id] = ss_ppu.io.bgpc[id];
    mmio.bgpd[id] = ss_ppu.io.bgpd[id];

    mmio.bgx[id].initial = (s32)ss_ppu.io.bgx[id];
    mmio.bgy[id].initial = (s32)ss_ppu.io.bgy[id];
    mmio.bgx[id]._current = ss_ppu.bgx[id].current;
    mmio.bgx[id].written = ss_ppu.bgx[id].written;
    mmio.bgy[id]._current = ss_ppu.bgy[id].current;
    mmio.bgy[id].written = ss_ppu.bgy[id].written;

    mmio.winh[id].WriteHalf(ss_ppu.io.winh[id]);
    mmio.winv[id].WriteHalf(ss_ppu.io.winv[id]);
  }

  mmio.winin.WriteHalf(ss_ppu.io.winin);
  mmio.winout.WriteHalf(ss_ppu.io.winout);

  mosaic.bg.size_x  = (ss_ppu.io.mosaic >>  0) & 15;
  mosaic.bg.size_y  = (ss_ppu.io.mosaic >>  4) & 15;
  mosaic.obj.size_x = (ss_ppu.io.mosaic >>  8) & 15;
  mosaic.obj.size_y =  ss_ppu.io.mosaic >> 12;

  mmio.bldcnt.WriteHalf(ss_ppu.io.bldcnt);
  mmio.eva = ss_ppu.io.bldalpha & 31;
  mmio.evb = (ss_ppu.io.bldalpha >> 8) & 31;
  mmio.evy = ss_ppu.io.bldy & 31;

  std::memcpy(pram, state.bus.memory.pram, 0x400);
  std::memcpy(oam,  state.bus.memory.oam,  0x400);
  std::memcpy(vram, state.bus.memory.vram, 0x18000);

  vram_bg_latch = ss_ppu.vram_bg_latch;
  dma3_video_transfer_running = ss_ppu.dma3_video_transfer_running;
}

void PPU::CopyState(SaveState& state) {
  auto& ss_ppu = state.ppu;
  auto& mosaic = mmio.mosaic;

  ss_ppu.io.dispcnt = mmio.dispcnt.ReadHalf();
  ss_ppu.io.greenswap = mmio.greenswap;
  ss_ppu.io.dispstat = mmio.dispstat.ReadHalf();
  ss_ppu.io.vcount = mmio.vcount;
  
  for(int id = 0; id < 4; id++) {
    ss_ppu.io.bgcnt[id] = mmio.bgcnt[id].ReadHalf();
    ss_ppu.io.bghofs[id] = mmio.bghofs[id];
    ss_ppu.io.bgvofs[id] = mmio.bgvofs[id];
  }

  for(int id = 0; id < 2; id++) {
    ss_ppu.io.bgpa[id] = mmio.bgpa[id];
    ss_ppu.io.bgpb[id] = mmio.bgpb[id];
    ss_ppu.io.bgpc[id] = mmio.bgpc[id];
    ss_ppu.io.bgpd[id] = mmio.bgpd[id];
    
    ss_ppu.io.bgx[id] = (u32)mmio.bgx[id].initial;
    ss_ppu.io.bgy[id] = (u32)mmio.bgy[id].initial;
    ss_ppu.bgx[id].current = mmio.bgx[id]._current;
    ss_ppu.bgx[id].written = mmio.bgx[id].written;
    ss_ppu.bgy[id].current = mmio.bgy[id]._current;
    ss_ppu.bgy[id].written = mmio.bgy[id].written;

    ss_ppu.io.winh[id] = mmio.winh[id].ReadHalf();
    ss_ppu.io.winv[id] = mmio.winv[id].ReadHalf();
  }

  ss_ppu.io.winin  = mmio.winin.ReadHalf();
  ss_ppu.io.winout = mmio.winout.ReadHalf();

  ss_ppu.io.mosaic = mosaic.bg.size_x  << 0 |
                     mosaic.bg.size_y  << 4 |
                     mosaic.obj.size_x << 8 |
                     mosaic.obj.size_y << 12;
  
  ss_ppu.io.bldcnt = mmio.bldcnt.ReadHalf();
  ss_ppu.io.bldalpha = mmio.eva | (mmio.evb << 8);
  ss_ppu.io.bldy = mmio.evy;

  std::memcpy(state.bus.memory.pram, pram, 0x400);
  std::memcpy(state.bus.memory.oam,  oam,  0x400);
  std::memcpy(state.bus.memory.vram, vram, 0x18000);

  ss_ppu.vram_bg_latch = vram_bg_latch;
  ss_ppu.dma3_video_transfer_running = dma3_video_transfer_running;
}

} // namespace nba::core