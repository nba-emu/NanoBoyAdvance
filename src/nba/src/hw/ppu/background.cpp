/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "ppu.hpp"

namespace nba::core {

void PPU::InitBackground() {
  bg.timestamp_last_sync = scheduler.GetTimestampNow();
  bg.cycle = 0;

  for(auto& text : bg.text) {
    text.fetching = false;
  }
}

void PPU::DrawBackground() {
  const u64 timestamp_now = scheduler.GetTimestampNow();
  
  int cycles = (int)(timestamp_now - bg.timestamp_last_sync);

  if(cycles == 0 || bg.cycle >= k_bg_cycle_limit) {
    return;
  }

  // fmt::print("PPU: draw background @ VCOUNT={} delta={}\n", mmio.vcount, cycles);

  const int mode = mmio.dispcnt.mode;

  switch(mode) {
    case 0: DrawBackgroundImpl<0>(cycles); break;
    case 1: DrawBackgroundImpl<1>(cycles); break;
    case 2: DrawBackgroundImpl<2>(cycles); break;
    case 3: DrawBackgroundImpl<3>(cycles); break;
    case 4: DrawBackgroundImpl<4>(cycles); break;
    case 5: DrawBackgroundImpl<5>(cycles); break;
    case 6: 
    case 7: DrawBackgroundImpl<7>(cycles); break;
  }

  bg.timestamp_last_sync = timestamp_now;
}

template<int mode> void PPU::DrawBackgroundImpl(int cycles) {
  for(int i = 0; i < cycles; i++) {
    // We add one to the cycle counter for convenience,
    // because it makes some of the timing math simpler.
    const uint cycle = 1U + bg.cycle;

    // text-mode backgrounds
    if constexpr(mode <= 1) {
      const uint id = cycle & 3U; // BG0 - BG3

      if((id <= 1 || mode == 0) && mmio.enable_bg[0][id]) {
        // @todo: make sure that the compiler will inline this call.
        RenderMode0BG(id, cycle);
      }
    }

    // affine backgrounds
    if constexpr(mode >= 1) {
      const int id = ~(cycle >> 1) & 1; // 0: BG2, 1: BG3

      // fmt::print("PPU: affine BG{} @ {}\n", id, bg.cycle);
    }

    // @todo: I don't think this is always correct, at least in text-mode.
    if(++bg.cycle == k_bg_cycle_limit) {
      break;
    }
  }
}

void PPU::RenderMode0BG(uint id, uint cycle) {
  const auto& bgcnt = mmio.bgcnt[id];

  auto& text = bg.text[id];

  const uint draw_x = cycle >> 2;

  // @todo: figure out if there is more logical way to control this condition
  if(text.fetching) {
    if(text.piso.remaining == 0) {
      u16 data = read<u16>(vram, text.tile.address);

      if(text.tile.flip_x) {
        data = (data >> 8) | (data << 8);

        if(!bgcnt.full_palette) {
          data = ((data & 0xF0F0U) >> 4) | ((data & 0x0F0FU) << 4);
        }

        text.tile.address -= sizeof(u16);
      } else {
        text.tile.address += sizeof(u16);
      }

      text.piso.data = data;
      text.piso.remaining = 4;
    }
  }

  uint index;

  if(bgcnt.full_palette) {
    index = text.piso.data & 0xFFU;

    text.piso.data >>= 8;
    text.piso.remaining -= 2;
  } else {
    index = text.piso.data & 0x0FU;

    if(index != 0U) {
      index |= text.tile.palette << 4;
    }

    text.piso.data >>= 4;
    text.piso.remaining--;
  }
  if(text.piso.remaining < 0) text.piso.remaining = 0;

  // quick output test
  if(id == 0) {
    const int test_x = draw_x - 9;

    if(test_x >= 0 && test_x < 240) {
      u16 color = read<u16>(pram, index << 1);
      uint r = (color >>  0) & 31U;
      uint g = (color >>  5) & 31U;
      uint b = (color >> 10) & 31U;

      output[frame][mmio.vcount * 240 + test_x] = 0xFF000000 | r << 19 | g << 11 | b << 3;
    }
  }
  
  const uint bghofs = mmio.bghofs[id];
  const uint bghofs_div_8 = bghofs >> 3;
  const uint bghofs_mod_8 = bghofs & 7U;

  // @todo: clean this mess up
  const uint test = draw_x + bghofs_mod_8;

  if(test >= 8 && (test & 7) == 0) {
    const u32 tile_base = bgcnt.tile_block << 14;
    /*const*/ uint map_block = bgcnt.map_block;

    /*const*/ uint line = mmio.vcount + mmio.bgvofs[id];

    if(bgcnt.mosaic_enable) {
      line -= (uint)mmio.mosaic.bg._counter_y;
    }

    const uint grid_x = bghofs_div_8 + (test >> 3) - 1U;
    const uint grid_y = line >> 3;
    const uint tile_y = line & 7U;

    const uint screen_x = (grid_x >> 5) & 1U;
    const uint screen_y = (grid_y >> 5) & 1U;

    switch(bgcnt.size) {
      case 1: map_block += screen_x; break;
      case 2: map_block += screen_y; break;
      case 3: map_block += screen_x + (screen_y << 1); break;
    }

    const u32 address = (map_block << 11) + ((grid_y & 31U) << 6) + ((grid_x & 31U) << 1);

    const u16 tile = read<u16>(vram, address);

    const uint number = tile & 0x3FFU;
    const bool flip_x = tile & (1U << 10);
    const bool flip_y = tile & (1U << 11);

    text.tile.palette = tile >> 12;
    text.tile.flip_x  = flip_x;

    const uint real_tile_y = flip_y ? (7 - tile_y) : tile_y;

    // @todo: research the low-level details of the address calculation.
    if(bgcnt.full_palette) {
      text.tile.address = tile_base + (number << 6) + (real_tile_y << 3);

      if(flip_x) {
        text.tile.address += 6;
      }
    } else {
      text.tile.address = tile_base + (number << 5) + (real_tile_y << 2);

      if(flip_x) {
        text.tile.address += 2;
      }
    }

    text.fetching = true;

    text.piso.remaining = 0;
  }
}

} // namespace nba::core
