/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "hw/ppu/ppu.hpp"
#include "hw/ppu/registers.hpp"

namespace nba::core {

void DisplayControl::Reset() {
  Write(0, 0);
  Write(1, 0);
}

auto DisplayControl::Read(int address) -> u8 {
  switch(address) {
    case 0: return (u8)(hword >> 0);
    case 1: return (u8)(hword >> 8);
  }

  return 0;
}

void DisplayControl::Write(int address, u8 value) {
  switch(address) {
    case 0: {
      hword = (hword & 0xFF00) | value;

      mode = value & 7;
      cgb_mode = (value >> 3) & 1;
      frame = (value >> 4) & 1;
      hblank_oam_access = (value >> 5) & 1;
      oam_mapping_1d = (value >> 6) & 1;
      forced_blank = (value >> 7) & 1;
      break;
    }
    case 1: {
      hword = (hword & 0x00FF) | (value << 8);

      for(int i = 0; i < 8; i++) {
        enable[i] = (value >> i) & 1;
      }
      break;
    }
  }
}

auto DisplayControl::ReadHalf() -> u16 {
  return Read(0) | (Read(1) << 8);
}

void DisplayControl::WriteHalf(u16 value) {
  Write(0, (u8)value);
  Write(1, (u8)(value >> 8));
}

void DisplayStatus::Reset() {
  Write(0, 0);
  Write(1, 0);
}

auto DisplayStatus::Read(int address) -> u8 {
  switch(address) {
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

void DisplayStatus::Write(int address, u8 value) {
  switch(address) {
    case 0:
      vblank_irq_enable = (value >> 3) & 1;
      hblank_irq_enable = (value >> 4) & 1;
      vcount_irq_enable = (value >> 5) & 1;
      break;
    case 1:
      vcount_setting = value;
      break;
  }

  ppu->UpdateVerticalCounterFlag();
}

auto DisplayStatus::ReadHalf() -> u16 {
  return Read(0) | (Read(1) << 8);
}

void DisplayStatus::WriteHalf(u16 value) {
  Write(0, (u8)value);
  Write(1, (u8)(value >> 8));
}

void BackgroundControl::Reset() {
  Write(0, 0);
  Write(1, 0);
}

auto BackgroundControl::Read(int address) -> u8 {
  switch(address) {
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

void BackgroundControl::Write(int address, u8 value) {  
  switch(address) {
    case 0:
      priority = value & 3;
      tile_block = (value >> 2) & 3;
      unused = (value >> 4) & 3;
      mosaic_enable = (value >> 6) & 1;
      full_palette = value >> 7;
      break;
    case 1:
      map_block = value & 0x1F;
      if(id >= 2) {
        wraparound = (value >> 5) & 1;
      }
      size = value >> 6;
      break;
  }
}

auto BackgroundControl::ReadHalf() -> u16 {
  return Read(0) | (Read(1) << 8);
}

void BackgroundControl::WriteHalf(u16 value) {
  Write(0, (u8)value);
  Write(1, (u8)(value >> 8));
}

void ReferencePoint::Reset() {
  initial = _current = 0;
  written = false;
}

void ReferencePoint::Write(int address, u8 value) {
  switch(address) {
    case 0: initial = (initial & 0x0FFFFF00) | (value <<  0); break;
    case 1: initial = (initial & 0x0FFF00FF) | (value <<  8); break;
    case 2: initial = (initial & 0x0F00FFFF) | (value << 16); break;
    case 3: initial = (initial & 0x00FFFFFF) | (value << 24); break;
  }
  
  if(initial & (1 << 27)) {
    initial |= 0xF0000000;
  }
  
  written = true;
}

void WindowRange::Reset() {
  min = 0;
  max = 0;
}

void WindowRange::Write(int address, u8 value) {
  switch(address) {
    case 0:
      max = value;
      break;
    case 1:
      min = value;
      break;
  }
}

auto WindowRange::ReadHalf() -> u16 {
  return max | (min << 8);
}

void WindowRange::WriteHalf(u16 value) {
  Write(0, (u8)value);
  Write(1, (u8)(value >> 8));
}

void WindowLayerSelect::Reset() {
  Write(0, 0);
  Write(1, 0);
}

auto WindowLayerSelect::Read(int address) -> u8 {
  u8 value = 0;
  
  for(int i = 0; i < 6; i++) {
    value |= enable[address][i] << i;
  }
  
  return value;
}

void WindowLayerSelect::Write(int address, u8 value) {
  for(int i = 0; i < 6; i++) {
    enable[address][i] = (value >> i) & 1;
  }
}

auto WindowLayerSelect::ReadHalf() -> u16 {
  return Read(0) | (Read(1) << 8);
}

void WindowLayerSelect::WriteHalf(u16 value) {
  Write(0, (u8)value);
  Write(1, (u8)(value >> 8));
}

void BlendControl::Reset() {
  Write(0, 0);
  Write(1, 0);
}

auto BlendControl::Read(int address) -> u8 {
  u8 value = 0;

  switch(address) {
    case 0:
      for(int i = 0; i < 6; i++)
        value |= targets[0][i] << i;
      value |= sfx << 6;
      break;
    case 1:
      for(int i = 0; i < 6; i++)
        value |= targets[1][i] << i;
      break;
  }

  return value;
}

void BlendControl::Write(int address, u8 value) {
  switch(address)
    case 0: {
      for(int i = 0; i < 6; i++)
        targets[0][i] = (value >> i) & 1;
      sfx = (Effect)(value >> 6);
      break;
    case 1:
      for(int i = 0; i < 6; i++)
        targets[1][i] = (value >> i) & 1;
      break;
  }
}

auto BlendControl::ReadHalf() -> u16 {
  return Read(0) | (Read(1) << 8);
}

void BlendControl::WriteHalf(u16 value) {
  Write(0, (u8)value);
  Write(1, (u8)(value >> 8));
}

void Mosaic::Reset() {
  bg.size_x = 1;
  bg.size_y = 1;
  bg._counter_y = 0;
  obj.size_x = 1;
  obj.size_y = 1;
  bg._counter_y = 0;
}

void Mosaic::Write(int address, u8 value) {
  // TODO: how does hardware handle mid-frame or mid-line mosaic changes?
  switch(address)
    case 0: {
      bg.size_x = (value & 15) + 1;
      bg.size_y = (value >> 4) + 1;
      break;
    case 1:
      obj.size_x = (value & 15) + 1;
      obj.size_y = (value >> 4) + 1;
      break;
  }
}

} // namespace nba::core
