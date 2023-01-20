/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

void ALWAYS_INLINE RenderMode0BG(uint id, uint cycle) {
  const auto& bgcnt = mmio.bgcnt[id];

  auto& text = bg.text[id];
  
  // @todo: figure out if there is more logical way to control this condition
  if(text.fetching) {
    if(text.piso.remaining == 0) {
      u16 data = read<u16>(vram, text.tile.address);

      // @todo: optimise VRAM access stall emulation
      bg.timestamp_vram_access = bg.timestamp_last_sync + cycle;

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

  const int screen_x = (cycle >> 2) - 9;

  if(screen_x >= 0 && screen_x < 240) {
    bg.buffer[screen_x][id] = index;
  }
  
  const uint bghofs = mmio.bghofs[id];
  const uint bghofs_div_8 = bghofs >> 3;
  const uint bghofs_mod_8 = bghofs & 7U;

  // @todo: find a better name for this?
  const uint step = (cycle >> 2) + bghofs_mod_8;

  if(step >= 8 && (step & 7) == 0) {
    const u32 tile_base = bgcnt.tile_block << 14;
    /*const*/ uint map_block = bgcnt.map_block;

    /*const*/ uint line = mmio.vcount + mmio.bgvofs[id];

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

    // @todo: optimise VRAM access stall emulation
    bg.timestamp_vram_access = bg.timestamp_last_sync + cycle;

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
    const u8 tile = vram[address];

    // @todo: optimise VRAM access stall emulation
    bg.timestamp_vram_access = bg.timestamp_last_sync + cycle;

    bg.affine[id].tile_address = (bgcnt.tile_block << 14) + (tile << 6) + ((y & 7) << 3) + (x & 7);
  } else {
    uint index = 0;

    if(!bg.affine[id].out_of_bounds) {
      index = vram[bg.affine[id].tile_address];
    }

    // Real hardware fetches VRAM even if the map coordinate is out-of-bounds.
    // @todo: optimise VRAM access stall emulation
    bg.timestamp_vram_access = bg.timestamp_last_sync + cycle;

    const uint x = (cycle - 32U) >> 2;

    // @todo: make the buffer larger and remove the condition.
    if(x < 240) {
      bg.buffer[x][2 + id] = index;
    }
  }
}