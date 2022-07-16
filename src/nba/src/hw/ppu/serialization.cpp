/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <cstring>

#include "ppu.hpp"

namespace nba::core {

// TODO: do not recreate the render thread on state load:
// this could get pretty slow with rewind.

void PPU::LoadState(SaveState const& state) {
  StopRenderThread();

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
  std::memcpy(state.ppu.pram, pram, 0x400);
  std::memcpy(state.ppu.oam,  oam,  0x400);
  std::memcpy(state.ppu.vram, vram, 0x18000);
}

} // namespace nba::core