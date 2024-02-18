/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/integer.hpp>

namespace nba::core {

class PPU;

struct DisplayControl {
  u16 hword;

  int mode;
  int cgb_mode;
  int frame;
  int hblank_oam_access;
  int oam_mapping_1d;
  int forced_blank;
  int enable[8];

  void Reset();
  auto Read(int address) -> u8;
  void Write(int address, u8 value);

  auto ReadHalf() -> u16;
  void WriteHalf(u16 value);

  PPU* ppu = nullptr;
};

struct DisplayStatus {
  int vblank_flag;
  int hblank_flag;
  int vcount_flag;
  int vblank_irq_enable;
  int hblank_irq_enable;
  int vcount_irq_enable;
  int vcount_setting;

  void Reset();
  auto Read(int address) -> u8;
  void Write(int address, u8 value);

  auto ReadHalf() -> u16;
  void WriteHalf(u16 value);

  PPU* ppu = nullptr;
};

struct BackgroundControl {
  int priority;
  int tile_block;
  int unused;
  int mosaic_enable;
  int full_palette;
  int map_block;
  int wraparound = false;
  int size;

  BackgroundControl(int id) : id(id) {}
  
  void Reset();
  auto Read(int address) -> u8;
  void Write(int address, u8 value);

  auto ReadHalf() -> u16;
  void WriteHalf(u16 value);

private:
  int id;
};

struct ReferencePoint {
  s32 initial;
  s32 _current;
  bool written;

  void Reset();
  void Write(int address, u8 value);
};
  
struct BlendControl {
  enum Effect {
    SFX_NONE,
    SFX_BLEND,
    SFX_BRIGHTEN,
    SFX_DARKEN
  } sfx;
  
  int targets[2][6];

  void Reset();
  auto Read(int address) -> u8;
  void Write(int address, u8 value);

  auto ReadHalf() -> u16;
  void WriteHalf(u16 value);
};

struct WindowRange {
  int min;
  int max;

  void Reset();
  void Write(int address, u8 value);

  auto ReadHalf() -> u16;
  void WriteHalf(u16 value);
};

struct WindowLayerSelect {
  int enable[2][6];

  void Reset();
  auto Read(int offset) -> u8;
  void Write(int offset, u8 value);

  auto ReadHalf() -> u16;
  void WriteHalf(u16 value);
};

struct Mosaic {
  struct {
    int size_x;
    int size_y;
    int _counter_y;
  } bg, obj;
  
  void Reset();
  void Write(int address, u8 value);
};

} // namespace nba::core
