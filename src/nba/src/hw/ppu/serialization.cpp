/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cstring>

#include "ppu.hpp"

namespace nba::core {

void PPU::LoadState(SaveState const& state) {
  /*
   * TODO: avoid quitting and recreating the render thread.
   * When we eventually implement rewind, this might be too slow.
   */
  StopRenderThread();

  mmio.dispcnt.WriteHalf(state.ppu.io.dispcnt);

  // NOTE: this must happen before DISPSTAT is written:
  mmio.vcount = state.ppu.io.vcount;

  u16 dispstat = state.ppu.io.dispstat;
  mmio.dispstat.vblank_flag = dispstat & 1;
  mmio.dispstat.hblank_flag = dispstat & 2;
  mmio.dispstat.vcount_flag = dispstat & 8;
  mmio.dispstat.WriteHalf(dispstat);

  for (int i = 0; i < 4; i++) {
    mmio.bgcnt[i].WriteHalf(state.ppu.io.bgcnt[i]);
    mmio.bghofs[i] = state.ppu.io.bghofs[i];
    mmio.bgvofs[i] = state.ppu.io.bgvofs[i];
  }

  for (int i = 0; i < 2; i++) {
    mmio.bgx[i].initial = state.ppu.io.bgx[i].initial;
    mmio.bgx[i]._current = state.ppu.io.bgx[i].current;
    mmio.bgx[i].written = state.ppu.io.bgx[i].written;

    mmio.bgy[i].initial = state.ppu.io.bgy[i].initial;
    mmio.bgy[i]._current = state.ppu.io.bgy[i].current;
    mmio.bgy[i].written = state.ppu.io.bgy[i].written;

    mmio.winh[i].WriteHalf(state.ppu.io.winh[i]);
    mmio.winv[i].WriteHalf(state.ppu.io.winv[i]);
    mmio.winh[i]._changed = true;
    mmio.winv[i]._changed = true;
  }

  mmio.winin.WriteHalf(state.ppu.io.winin);
  mmio.winout.WriteHalf(state.ppu.io.winout);

  mmio.mosaic.bg.size_x = state.ppu.io.mosaic.bg.size_x;
  mmio.mosaic.bg.size_y = state.ppu.io.mosaic.bg.size_y;
  mmio.mosaic.bg._counter_y = state.ppu.io.mosaic.bg.counter_y;
  mmio.mosaic.obj.size_x = state.ppu.io.mosaic.obj.size_x;
  mmio.mosaic.obj.size_y = state.ppu.io.mosaic.obj.size_y;
  mmio.mosaic.obj._counter_y = state.ppu.io.mosaic.obj.counter_y;

  mmio.bldcnt.WriteHalf(state.ppu.io.bldcnt);
  mmio.eva = state.ppu.io.bldalpha & 0xFF;
  mmio.evb = state.ppu.io.bldalpha >> 8;
  mmio.evy = state.ppu.io.bldy;

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      mmio.enable_bg[i][j] = state.ppu.enable_bg[i][j];
    }
    window_scanline_enable[i] = state.ppu.window_scanline_enable[i];
  }

  std::memcpy(pram, state.ppu.pram, 0x400);
  std::memcpy(oam,  state.ppu.oam,  0x400);
  std::memcpy(vram, state.ppu.vram, 0x18000);

  std::memcpy(pram_draw, state.ppu.pram, 0x400);
  std::memcpy(oam_draw,  state.ppu.oam,  0x400);
  std::memcpy(vram_draw, state.ppu.vram, 0x18000);
  vram_dirty_range_lo = 0x18000;
  vram_dirty_range_hi = 0;

  SetupRenderThread();
}

void PPU::CopyState(SaveState& state) {
  state.ppu.io.dispcnt = mmio.dispcnt.ReadHalf();
  state.ppu.io.dispstat = mmio.dispstat.ReadHalf();
  state.ppu.io.vcount = mmio.vcount;

  for (int i = 0; i < 4; i++) {
    state.ppu.io.bgcnt[i] = mmio.bgcnt[i].ReadHalf();
    state.ppu.io.bghofs[i] = mmio.bghofs[i];
    state.ppu.io.bgvofs[i] = mmio.bgvofs[i];
  }

  for (int i = 0; i < 2; i++) {
    state.ppu.io.bgx[i].initial = mmio.bgx[i].initial;
    state.ppu.io.bgx[i].current = mmio.bgx[i]._current;
    state.ppu.io.bgx[i].written = mmio.bgx[i].written;

    state.ppu.io.bgy[i].initial = mmio.bgy[i].initial;
    state.ppu.io.bgy[i].current = mmio.bgy[i]._current;
    state.ppu.io.bgy[i].written = mmio.bgy[i].written;

    state.ppu.io.winh[i] = mmio.winh[i].ReadHalf();
    state.ppu.io.winv[i] = mmio.winv[i].ReadHalf();
  }

  state.ppu.io.winin = mmio.winin.ReadHalf();
  state.ppu.io.winout = mmio.winout.ReadHalf();

  state.ppu.io.mosaic.bg.size_x = mmio.mosaic.bg.size_x;
  state.ppu.io.mosaic.bg.size_y = mmio.mosaic.bg.size_y;
  state.ppu.io.mosaic.bg.counter_y = mmio.mosaic.bg._counter_y;
  state.ppu.io.mosaic.obj.size_x = mmio.mosaic.obj.size_x;
  state.ppu.io.mosaic.obj.size_y = mmio.mosaic.obj.size_y;
  state.ppu.io.mosaic.obj.counter_y = mmio.mosaic.obj._counter_y;

  state.ppu.io.bldcnt = mmio.bldcnt.ReadHalf();
  state.ppu.io.bldalpha = mmio.eva | (mmio.evb << 8);
  state.ppu.io.bldy = mmio.evy;

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 4; j++) {
      state.ppu.enable_bg[i][j] = mmio.enable_bg[i][j];
    }
    state.ppu.window_scanline_enable[i] = window_scanline_enable[i];
  }

  std::memcpy(state.ppu.pram, pram, 0x400);
  std::memcpy(state.ppu.oam,  oam,  0x400);
  std::memcpy(state.ppu.vram, vram, 0x18000);
}

} // namespace nba::core