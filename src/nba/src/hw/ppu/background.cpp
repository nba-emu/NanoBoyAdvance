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
  bg.x = 0;
  bg.hcounter = 0;

  if (dispcnt_mode != 0 && id >= 2) { 
    // TODO: rename _current to current:
    bg.affine.x = mmio.bgx[id - 2]._current;
    bg.affine.y = mmio.bgy[id - 2]._current;
  }
}

void PPU::SyncBG(int id, int cycles) {
  // TODO: optimize dispatch of BG mode specific function:
  switch (dispcnt_mode) {
    case 1: if (id == 2) RenderBGMode2(id, cycles); break;
    case 2: RenderBGMode2(id, cycles); break;
    case 3: RenderBGMode3(cycles); break;
    case 4: RenderBGMode4(cycles); break;
    case 5: RenderBGMode5(cycles); break;
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
 