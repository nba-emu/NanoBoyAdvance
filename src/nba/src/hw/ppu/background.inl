/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

void ALWAYS_INLINE RenderMode0BG(uint id, uint cycle) {
  const auto& bgcnt = mmio.bgcnt[id];

  auto& text = bg.text[id];
  
  // @todo: figure out if there is more logical way to control this condition
  if(text.fetches > 0 && text.piso.remaining == 0) {
    u16 data = FetchVRAM_BG<u16>(cycle, text.tile.address);

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

    text.fetches--;
  }

  uint index;

  const int screen_x = (cycle >> 2) - 9;

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

  if(screen_x >= 0 && screen_x < 240) {
    bg.buffer[screen_x][id] = index;
  }

  const uint bghofs = mmio.bghofs[id];
  const uint bghofs_div_8 = bghofs >> 3;
  const uint bghofs_mod_8 = bghofs & 7U;

  // @todo: find a better name for this?
  const uint step = (cycle >> 2) + bghofs_mod_8;

  if(cycle < 1007U && step >= 8 && (step & 7) == 0) {
    const u32 tile_base = bgcnt.tile_block << 14;
    uint map_block = bgcnt.map_block;

    uint line = mmio.vcount + mmio.bgvofs[id];

    if(bgcnt.mosaic_enable) {
      line -= (uint)mmio.mosaic.bg._counter_y;
    }

    const uint grid_x = bghofs_div_8 + (step >> 3) - 1U;
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

    const u16 tile = FetchVRAM_BG<u16>(cycle, address);

    /**
     * In cycles 1004 - 1006 map fetches can still happen,
     * however curiously no tile fetches follow.
     * If a map fetch were to happen in cycle 1003 though,
     * tile fetches would follow even during H-blank?
     */
    if(cycle < 1004U) {
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

        text.fetches = 4;
      } else {
        text.tile.address = tile_base + (number << 5) + (real_tile_y << 2);

        if(flip_x) {
          text.tile.address += 2;
        }

        text.fetches = 2;
      }

      text.piso.remaining = 0;
    }
  }
}

void ALWAYS_INLINE RenderMode2BG(uint id, uint cycle) {
  const auto& bgcnt = mmio.bgcnt[2 + id];

  if(cycle < 32U) {
    return;
  }

  if((cycle & 1U) == 0U) {
    const int log_size = bgcnt.size;
    const s32 size = 128 << log_size;
    const s32 mask = size - 1;

    s32 x = bg.affine[id].x >> 8;
    s32 y = bg.affine[id].y >> 8;

    // @todo: figure out in what cycle this happens.
    bg.affine[id].x += mmio.bgpa[id];
    bg.affine[id].y += mmio.bgpc[id];

    if(bgcnt.wraparound) {
      x &= mask;
      y &= mask;
      bg.affine[id].out_of_bounds = false;
    } else {
      // disable fetches if either X or Y is outside the [0, size) range.
      bg.affine[id].out_of_bounds = ((x | y) & -size) != 0;
    }

    const u16 address = (bgcnt.map_block << 11) + ((y >> 3) << (4 + log_size)) + (x >> 3);
    const u8 tile = FetchVRAM_BG<u8>(cycle, address);

    bg.affine[id].tile_address = (bgcnt.tile_block << 14) + (tile << 6) + ((y & 7) << 3) + (x & 7);
  } else {
    uint index = FetchVRAM_BG<u8>(cycle, bg.affine[id].tile_address);

    if(bg.affine[id].out_of_bounds) {
      index = 0U;
    }

    const uint x = (cycle - 32U) >> 2;

    // @todo: make the buffer larger and remove the condition.
    if(x < 240) {
      bg.buffer[x][2 + id] = index;
    }
  }
}

void ALWAYS_INLINE RenderMode3BG(uint cycle) {
  if(cycle < 32U || (cycle & 3U) != 3U) {
    return;
  }

  const uint screen_x = (cycle - 32U) >> 2;

  const s32 x = bg.affine[0].x >> 8;
  const s32 y = bg.affine[0].y >> 8;

  /**
   * @todo: confirm that the address actually is 17-bit.
   * This should matter only for open bus shenanigans when switching BG mode mid-scanline.
   */
  const u32 address = ((u32)y * 240U + (u32)x) * 2U;
  const u16 data = FetchVRAM_BG<u16>(cycle, address & 0x1FFFFU);

  u32 color = 0U;

  if(x >= 0 && x < 240 && y >= 0 && y < 160) {
    color = data | 0x8000'0000;
  }

  if(screen_x < 240U) {
    bg.buffer[screen_x][2] = color;
  }

  bg.affine[0].x += mmio.bgpa[0];
  bg.affine[0].y += mmio.bgpc[0];
}

void ALWAYS_INLINE RenderMode4BG(uint cycle) {
  if(cycle < 32U || (cycle & 3U) != 3U) {
    return;
  }

  const uint screen_x = (cycle - 32U) >> 2;

  const s32 x = bg.affine[0].x >> 8;
  const s32 y = bg.affine[0].y >> 8;

  /**
   * @todo: confirm that the address actually is 17-bit.
   * This should matter only for open bus shenanigans when switching BG mode mid-scanline.
   */
  const u32 address = mmio.dispcnt.frame * 0xA000U + (u32)y * 240U + (u32)x;
  const u8  data = FetchVRAM_BG<u8>(cycle, address & 0x1FFFFU);

  uint index = 0U;

  if(x >= 0 && x < 240 && y >= 0 && y < 160) {
    index = data;
  }

  if(screen_x < 240U) {
    bg.buffer[screen_x][2] = index;
  }

  bg.affine[0].x += mmio.bgpa[0];
  bg.affine[0].y += mmio.bgpc[0];
}

void ALWAYS_INLINE RenderMode5BG(uint cycle) {
  if(cycle < 32U || (cycle & 3U) != 3U) {
    return;
  }

  const uint screen_x = (cycle - 32U) >> 2;

  const s32 x = bg.affine[0].x >> 8;
  const s32 y = bg.affine[0].y >> 8;

  /**
   * @todo: confirm that the address actually is 17-bit.
   * This should matter only for open bus shenanigans when switching BG mode mid-scanline.
   */
  const u32 address = mmio.dispcnt.frame * 0xA000U + ((u32)y * 160U + (u32)x) * 2U;
  const u16 data = FetchVRAM_BG<u16>(cycle, address & 0x1FFFFU);

  u32 color = 0U;

  if(x >= 0 && x < 160 && y >= 0 && y < 128) {
    color = data | 0x8000'0000;
  }

  if(screen_x < 240U) {
    bg.buffer[screen_x][2] = color;
  }

  bg.affine[0].x += mmio.bgpa[0];
  bg.affine[0].y += mmio.bgpc[0];
}