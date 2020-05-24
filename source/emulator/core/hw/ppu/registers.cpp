/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "ppu.hpp"
#include "registers.hpp"

namespace nba::core {

void DisplayControl::Reset() {
  Write(0, 0);
  Write(1, 0);
}

auto DisplayControl::Read(int address) -> std::uint8_t {
  switch (address) {
  case 0:
    return mode |
          (cgb_mode << 3) |
          (frame << 4) |
          (hblank_oam_access << 5) |
          (oam_mapping_1d << 6) |
          (forced_blank << 7);
  case 1:
    return enable[0] |
          (enable[1] << 1) |
          (enable[2] << 2) |
          (enable[3] << 3) |
          (enable[4] << 4) |
          (enable[5] << 5) |
          (enable[6] << 6) |
          (enable[7] << 7);
  }

  return 0;
}

void DisplayControl::Write(int address, std::uint8_t value) {
  switch (address) {
  case 0: {
    auto mode_new = value & 7;
    _mode_is_dirty = mode != mode_new;
    mode = mode_new;
    cgb_mode = (value >> 3) & 1;
    frame = (value >> 4) & 1;
    hblank_oam_access = (value >> 5) & 1;
    oam_mapping_1d = (value >> 6) & 1;
    forced_blank = (value >> 7) & 1;
    break;
  }
  case 1: {
    for (int i = 0; i < 8; i++) {
      enable[i] = (value >> i) & 1;
    }
    break;
  }
  }
}

void DisplayStatus::Reset() {
  Write(0, 0);
  Write(1, 0);
}

auto DisplayStatus::Read(int address) -> std::uint8_t {
  switch (address) {
  case 0:
    return vblank_flag |
          (hblank_flag << 1) |
          (vcount_flag << 2) |
          (vblank_irq_enable << 3) |
          (hblank_irq_enable << 4) |
          (vcount_irq_enable << 5);
  case 1:
    return vcount_setting;
  }

  return 0;
}

void DisplayStatus::Write(int address, std::uint8_t value) {
  switch (address) {
  case 0:
    vblank_irq_enable = (value >> 3) & 1;
    hblank_irq_enable = (value >> 4) & 1;
    vcount_irq_enable = (value >> 5) & 1;
    if (ppu != nullptr) {
      ppu->CheckForVcountIRQ();
    }
    break;
  case 1:
    vcount_setting = value;
    if (ppu != nullptr) {
      ppu->CheckForVcountIRQ();
    }
    break;
  }
}

void BackgroundControl::Reset() {
  Write(0, 0);
  Write(1, 0);
}

auto BackgroundControl::Read(int address) -> std::uint8_t {
  switch (address) {
  case 0:
    return priority |
          (tile_block << 2) |
          (unused << 4) |
          (mosaic_enable << 6) |
          (full_palette << 7);
  case 1:
    return map_block |
          (wraparound << 5) |
          (size << 6);
  }

  return 0;
}

void BackgroundControl::Write(int address, std::uint8_t value) {  
  switch (address) {
  case 0:
    priority = value & 3;
    tile_block = (value >> 2) & 3;
    unused = (value >> 4) & 3;
    mosaic_enable = (value >> 6) & 1;
    full_palette = value >> 7;
    break;
  case 1:
    map_block = value & 0x1F;
    if (id >= 2) {
      wraparound = (value >> 5) & 1;
    }
    size = value >> 6;
    break;
  }
}

void ReferencePoint::Reset() {
  initial = _current = 0;
}

void ReferencePoint::Write(int address, std::uint8_t value) {
  switch (address) {
  case 0: initial = (initial & 0x0FFFFF00) | (value << 0); break;
  case 1: initial = (initial & 0x0FFF00FF) | (value << 8); break;
  case 2: initial = (initial & 0x0F00FFFF) | (value << 16); break;
  case 3: initial = (initial & 0x00FFFFFF) | (value << 24); break;
  }
  
  if (initial & (1 << 27)) {
    initial |= 0xF0000000;
  }
  
  _current = initial;
}

void WindowRange::Reset() {
  min = 0;
  max = 0;
  _changed = false;
}

void WindowRange::Write(int address, std::uint8_t value) {
  switch (address) {
  case 0:
    if (value != max) _changed = true;
    max = value;
    break;
  case 1:
    if (value != min) _changed = true;
    min = value;
    break;
  }
}

void WindowLayerSelect::Reset() {
  Write(0, 0);
  Write(1, 0);
}

auto WindowLayerSelect::Read(int address) -> std::uint8_t {
  std::uint8_t value = 0;
  
  for (int i = 0; i < 6; i++) {
    value |= enable[address][i] << i;
  }
  
  return value;
}

void WindowLayerSelect::Write(int address, std::uint8_t value) {
  for (int i = 0; i < 6; i++) {
    enable[address][i] = (value >> i) & 1;
  }
}

void BlendControl::Reset() {
  Write(0, 0);
  Write(1, 0);
}

auto BlendControl::Read(int address) -> std::uint8_t {
  switch (address) {
  case 0: {
    std::uint8_t value = 0;
    for (int i = 0; i < 6; i++)
      value |= targets[0][i] << i;
    value |= sfx << 6;
    return value;
  }
  case 1: {
    std::uint8_t value = 0;
    for (int i = 0; i < 6; i++)
      value |= targets[1][i] << i;
    return value;
  }
  }

  return 0;
}

void BlendControl::Write(int address, std::uint8_t value) {
  switch (address) {
  case 0: {
    for (int i = 0; i < 6; i++)
      targets[0][i] = (value >> i) & 1;
    sfx = (Effect)(value >> 6);
    break;
  }
  case 1: {
    for (int i = 0; i < 6; i++)
      targets[1][i] = (value >> i) & 1;
    break;
  }
  }
}

void Mosaic::Reset() {
  bg.size_x = 1;
  bg.size_y = 1;
  bg._counter_y = 0;
  obj.size_x = 1;
  obj.size_y = 1;
  bg._counter_y = 0;
}

void Mosaic::Write(int address, std::uint8_t value) {
  switch (address) {
  case 0: {
    bg.size_x = (value & 15) + 1;
    bg.size_y = (value >> 4) + 1;
    
    /* TODO: find out if/how the hardware does this. */
    bg._counter_y = 0;
    break;
  }
  case 1: {
    obj.size_x = (value & 15) + 1;
    obj.size_y = (value >> 4) + 1;
    
    /* TODO: find out if/how the hardware does this. */
    obj._counter_y = 0;
    break;
  }
  }
}

} // namespace nba::core
