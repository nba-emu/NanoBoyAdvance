/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "hw/ppu/ppu.hpp"

namespace nba::core {

// TODO: !!! deduplicate Mode3/4/5 code in a clean way!!!

void PPU::InitBG(int id) {
  auto& bg = this->bg[id];

  bg.engaged = true;
  bg.hcounter = 0;

  if (dispcnt_mode != 0 && id >= 2) { 
    // TODO: rename _current to current:
    bg.affine.x = mmio.bgx[id - 2]._current;
    bg.affine.y = mmio.bgy[id - 2]._current;
  }

  // Text-mode:
  if (dispcnt_mode == 0 || (dispcnt_mode == 1 && id < 2)) {
    bg.x = -(mmio.bghofs[id] & 7); // sigh, we write to x twice
    bg.text.grid_x = 0;
  } else {
    bg.x = 0;
  }
}

void PPU::SyncBG(int id, int cycles) {
  // TODO: optimize dispatch of BG mode specific function:
  switch (dispcnt_mode) {
    case 0: RenderBGMode0(id, cycles); break;
    case 1: (id == 2) ? RenderBGMode2(id, cycles) : RenderBGMode0(id, cycles); break;
    case 2: RenderBGMode2(id, cycles); break;
    case 3: RenderBGMode3(cycles); break;
    case 4: RenderBGMode4(cycles); break;
    case 5: RenderBGMode5(cycles); break;
  }
}

void PPU::FetchMapMode0(int id) {
  auto& bg = this->bg[id];
  auto& bgcnt = mmio.bgcnt[id];

  // TODO: should BGXCNT be latched?
  u32 tile_base = bgcnt.tile_block << 14;
  int map_block = bgcnt.map_block;

  // TODO: should BGXHOFS (possibly only the lower three bits?) and BGXVOFS be latched?
  int line   = mmio.bgvofs[id] + mmio.vcount;
  int grid_x = (mmio.bghofs[id] >> 3) + bg.text.grid_x;
  int grid_y = line >> 3;
  int tile_y = line & 7;

  auto screen_x = (grid_x >> 5) & 1;
  auto screen_y = (grid_y >> 5) & 1;

  switch (bgcnt.size) {
    case 1:
      map_block += screen_x;
      break;
    case 2:
      map_block += screen_y;
      break;
    case 3:
      map_block += screen_x;
      map_block += screen_y << 1;
      break;
  }

  u32 address = (map_block << 11) + ((grid_y & 31) << 6) + ((grid_x & 31) << 1);

  u16 map_entry = read<u16>(vram, address);
  int number = map_entry & 0x3FF;
  int palette = map_entry >> 12;
  bool flip_x = map_entry & (1 << 10);
  bool flip_y = map_entry & (1 << 11);

  if (flip_y) tile_y ^= 7;

  bg.text.palette = palette;
  bg.text.full_palette = bgcnt.full_palette;
  bg.text.flip_x = flip_x;

  if (bgcnt.full_palette) {
    bg.text.address = tile_base + (number << 6) + (tile_y << 3);
    if (flip_x) {
      bg.text.address += 6;
    }
  } else {
    bg.text.address = tile_base + (number << 5) + (tile_y << 2);
    if (flip_x) {
      bg.text.address += 2;
    }
  }
}

void PPU::FetchTileMode04BPP(int id) {
  auto& bg = this->bg[id];

  u16 data = read<u16>(vram, bg.text.address);
  int flip = bg.text.flip_x ? 3 : 0;
  int draw_x = bg.x;
  int palette = bg.text.palette;

  for (int x = 0; x < 4; x++) {
    u32 index = (data & 15);

    if (index == 0) {
      index = 0x8000'0000;
    } else {
      index |= palette << 4;
    }

    bg.buffer[8 + draw_x + (x ^ flip)] = index;
    data >>= 4;
  }

  bg.x += 4;

  if (bg.text.flip_x) {
    bg.text.address -= sizeof(u16);
  } else {
    bg.text.address += sizeof(u16);
  }
}

void PPU::FetchTileMode08BPP(int id) {
  auto& bg = this->bg[id];

  u16 data = read<u16>(vram, bg.text.address);
  int flip = bg.text.flip_x ? 1 : 0;
  int draw_x = bg.x;

  for (int x = 0; x < 2; x++) {
    u32 index = data & 0xFF;

    if (index == 0) {
      index = 0x8000'0000;
    }

    bg.buffer[8 + draw_x + (x ^ flip)] = index;
    data >>= 8;
  }

  bg.x += 2;

  if (bg.text.flip_x) {
    bg.text.address -= sizeof(u16);
  } else {
    bg.text.address += sizeof(u16);
  }
}

void PPU::RenderBGMode0(int id, int cycles) {
  const int RENDER_DELAY = 34 + id;

  auto& bg = this->bg[id];
  auto& bgcnt = mmio.bgcnt[id];

  int hcounter = bg.hcounter;
  int hcounter_target = hcounter + cycles;

  bg.hcounter = hcounter_target;

  hcounter = std::max(hcounter, RENDER_DELAY);

  while (hcounter < hcounter_target) {
    int cycle = (hcounter - RENDER_DELAY) & 31;

    int hcounter_next = hcounter + 1;

    // TODO: can this logic be meaningfully optimized more?
    if (cycle == 0) {
      FetchMapMode0(id);
      hcounter_next = hcounter + 4;
    } else {
      if (bg.text.full_palette) {
        switch (cycle) {
          case  4:
          case 12:
          case 20: FetchTileMode08BPP(id); hcounter_next = hcounter + 8; break;
          case 28: FetchTileMode08BPP(id); hcounter_next = hcounter + 4; break;
        }
      } else {
        // TODO: we can get rid of one sync-point by updating grid_x in cycle 20 (only in 4BPP mode).
        switch (cycle) {
          case  4: FetchTileMode04BPP(id); hcounter_next = hcounter + 16; break;
          case 20: FetchTileMode04BPP(id); hcounter_next = hcounter +  8; break;
        }
      }

      if (cycle == 28) {
        // TODO: stop at 30 tiles if (BGHOFS & 7) == 0:
        if (++bg.text.grid_x == 31) {
          bg.engaged = false;
          break;
        }
      }
    }

    hcounter = hcounter_next;
  }
}

void PPU::RenderBGMode2(int id, int cycles) {
  const int RENDER_DELAY = 36 + ((id - 2) * 2);

  auto& bg = this->bg[id];
  auto& bgcnt = mmio.bgcnt[id];

  int hcounter = bg.hcounter;
  int hcounter_target = hcounter + cycles;

  bg.hcounter = hcounter_target;

  s16 pa = mmio.bgpa[id - 2];
  s16 pc = mmio.bgpc[id - 2];
  int log_size = bgcnt.size;
  int size = 128 << log_size;
  int mask = size - 1;
  u32 map_base  = bgcnt.map_block << 11;
  u32 tile_base = bgcnt.tile_block << 14;
  bool wraparound = bgcnt.wraparound;

  hcounter = std::max(hcounter, RENDER_DELAY);

  u32* buffer = &bg.buffer[8 + bg.x];

  while (hcounter < hcounter_target) {
    int cycle = (hcounter - RENDER_DELAY) & 3;

    // TODO: simplify this logic?
    if (cycle == 0) {
      s32 x = bg.affine.x >> 8;
      s32 y = bg.affine.y >> 8;

      bg.affine.x += pa;
      bg.affine.y += pc;

      if (wraparound) {
        x &= mask;
        y &= mask;
        bg.affine.fetching = true;
      } else {
        // disable fetches if either X or Y is outside the [0, size) range.
        bg.affine.fetching = ((x | y) & -size) == 0;
      }

      if (bg.affine.fetching) {
        u32 map_address = map_base + ((y >> 3) << (4 + log_size)) + (x >> 3);
        int tile = vram[map_address];

        bg.affine.address = tile_base + (tile << 6) + ((y & 7) << 3) + (x & 7);
      }

      hcounter++;
    } else if (cycle == 1) {
      u32 pixel = 0x8000'0000;

      if (bg.affine.fetching) {
        pixel = vram[bg.affine.address];

        if (pixel == 0) {
          pixel = 0x8000'0000;
        }
      }

      *buffer++ = pixel;
    
      hcounter += 3;

      if (++bg.x == 240) {
        bg.engaged = false;
        break;
      }
    } else {
      hcounter += 4 - cycle;
    }
  }
}

void PPU::RenderBGMode3(int cycles) {
  const int RENDER_DELAY = 37;

  auto& bg = this->bg[2];

  int hcounter = bg.hcounter;
  int hcounter_target = hcounter + cycles;

  bg.hcounter = hcounter_target;

  s16 pa = mmio.bgpa[0];
  s16 pc = mmio.bgpc[0];

  hcounter = std::max(hcounter, RENDER_DELAY);

  u32* buffer = &bg.buffer[8 + bg.x];

  while (hcounter < hcounter_target) {
    int cycle = (hcounter - RENDER_DELAY) & 3;

    if (cycle == 0) {
      u32 x = bg.affine.x >> 8;
      u32 y = bg.affine.y >> 8;

      bg.affine.x += pa;
      bg.affine.y += pc;

      u32 pixel = 0x8000'0000;

      if (x < 240 && y < 160) {
        u32 address = (y * 240 + x) * 2;

        pixel = read<u16>(vram, address) | 0x4000'0000;
      }

      *buffer++ = pixel;

      hcounter += 4;

      if (++bg.x == 240) {
        bg.engaged = false;
        break;
      }
    } else {
      hcounter += 4 - cycle;
    }
  }
}

void PPU::RenderBGMode4(int cycles) {
  const int RENDER_DELAY = 37;

  auto& bg = this->bg[2];

  int hcounter = bg.hcounter;
  int hcounter_target = hcounter + cycles;

  bg.hcounter = hcounter_target;

  s16 pa = mmio.bgpa[0];
  s16 pc = mmio.bgpc[0];
  // TODO: should DISPCNT.frame be latched?
  u32 base_address = mmio.dispcnt.frame * 0xA000;

  hcounter = std::max(hcounter, RENDER_DELAY);

  u32* buffer = &bg.buffer[8 + bg.x];

  while (hcounter < hcounter_target) {
    int cycle = (hcounter - RENDER_DELAY) & 3;

    if (cycle == 0) {
      u32 x = bg.affine.x >> 8;
      u32 y = bg.affine.y >> 8;

      bg.affine.x += pa;
      bg.affine.y += pc;

      u32 pixel = 0x8000'0000;

      if (x < 240 && y < 160) {
        pixel = vram[base_address + (y * 240) + x];
        if (pixel == 0) {
          pixel = 0x8000'0000;
        }
      }

      *buffer++ = pixel;

      hcounter += 4;

      if (++bg.x == 240) {
        bg.engaged = false;
        break;
      }
    } else {
      hcounter += 4 - cycle;
    }
  }
}

void PPU::RenderBGMode5(int cycles) {
  const int RENDER_DELAY = 37;

  auto& bg = this->bg[2];

  int hcounter = bg.hcounter;
  int hcounter_target = hcounter + cycles;

  bg.hcounter = hcounter_target;

  s16 pa = mmio.bgpa[0];
  s16 pc = mmio.bgpc[0];
  // TODO: should DISPCNT.frame be latched?
  u32 base_address = mmio.dispcnt.frame * 0xA000;

  hcounter = std::max(hcounter, RENDER_DELAY);

  u32* buffer = &bg.buffer[8 + bg.x];

  while (hcounter < hcounter_target) {
    int cycle = (hcounter - RENDER_DELAY) & 3;

    if (cycle == 0) {
      u32 x = bg.affine.x >> 8;
      u32 y = bg.affine.y >> 8;

      bg.affine.x += pa;
      bg.affine.y += pc;

      u32 pixel = 0x8000'0000;

      if (x < 160 && y < 128) {
        u32 address = base_address + (y * 160 + x) * 2;

        pixel = read<u16>(vram, address) | 0x4000'0000;
      }

      *buffer++ = pixel;

      hcounter += 4;

      if (++bg.x == 240) {
        bg.engaged = false;
        break;
      }
    } else {
      hcounter += 4 - cycle;
    }
  }
}

} // namespace nba::core
 