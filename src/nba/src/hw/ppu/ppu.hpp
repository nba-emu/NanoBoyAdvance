/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/deprecate/config.hpp>
#include <nba/integer.hpp>

#include <nba/common/compiler.hpp>
#include <nba/common/punning.hpp>
#include <hw/dma.hpp>
#include <hw/interrupt.hpp>
#include <scheduler.hpp>
#include <functional>
#include <type_traits>

#include "registers.hpp"

namespace nba::core {

struct PPU {
  PPU(
    Scheduler& scheduler,
    IRQ& irq,
    DMA& dma,
    std::shared_ptr<Config> config
  );

  void Reset();

  template<typename T>
  auto ALWAYS_INLINE ReadPRAM(u32 address) noexcept -> T {
    return read<T>(pram, address & 0x3FF);
  }

  template<typename T>
  void ALWAYS_INLINE WritePRAM(u32 address, T value) noexcept {
    if constexpr (std::is_same_v<T, u8>) {
      write<u16>(pram, address & 0x3FE, value * 0x0101);
    } else {
      write<T>(pram, address & 0x3FF, value);
    }
  }

  template<typename T>
  auto ALWAYS_INLINE ReadVRAM(u32 address) noexcept -> T {
    address &= 0x1FFFF;
    if (address >= 0x18000) {
      address &= ~0x8000;
    }
    return read<T>(vram, address);
  }

  template<typename T>
  void ALWAYS_INLINE WriteVRAM(u32 address, T value) noexcept {
    address &= 0x1FFFF;
    if (address >= 0x18000) {
      address &= ~0x8000;
    }
    if (std::is_same_v<T, u8>) {
      auto limit = mmio.dispcnt.mode >= 3 ? 0x14000 : 0x10000;
      if (address < limit) {
        write<u16>(vram, address & ~1, value * 0x0101);
      }
    } else {
      write<T>(vram, address, value);
    }
  }

  template<typename T>
  auto ALWAYS_INLINE ReadOAM(u32 address) noexcept -> T {
    return read<T>(oam, address & 0x3FF);
  }

  template<typename T>
  void ALWAYS_INLINE WriteOAM(u32 address, T value) noexcept {
    if constexpr (!std::is_same_v<T, u8>) {
      write<T>(oam, address & 0x3FF, value);
    }
  }

  struct MMIO {
    DisplayControl dispcnt;
    DisplayStatus dispstat;

    u8 vcount;

    BackgroundControl bgcnt[4] { 0, 1, 2, 3 };

    u16 bghofs[4];
    u16 bgvofs[4];

    ReferencePoint bgx[2], bgy[2];
    s16 bgpa[2];
    s16 bgpb[2];
    s16 bgpc[2];
    s16 bgpd[2];

    WindowRange winh[2];
    WindowRange winv[2];
    WindowLayerSelect winin;
    WindowLayerSelect winout;

    Mosaic mosaic;

    BlendControl bldcnt;
    int eva;
    int evb;
    int evy;
  } mmio;

private:
  friend struct DisplayStatus;

  enum ObjAttribute {
    OBJ_IS_ALPHA  = 1,
    OBJ_IS_WINDOW = 2
  };

  enum ObjectMode {
    OBJ_NORMAL = 0,
    OBJ_SEMI   = 1,
    OBJ_WINDOW = 2,
    OBJ_PROHIBITED = 3
  };

  enum Layer {
    LAYER_BG0 = 0,
    LAYER_BG1 = 1,
    LAYER_BG2 = 2,
    LAYER_BG3 = 3,
    LAYER_OBJ = 4,
    LAYER_SFX = 5,
    LAYER_BD  = 5
  };

  enum Enable {
    ENABLE_BG0 = 0,
    ENABLE_BG1 = 1,
    ENABLE_BG2 = 2,
    ENABLE_BG3 = 3,
    ENABLE_OBJ = 4,
    ENABLE_WIN0 = 5,
    ENABLE_WIN1 = 6,
    ENABLE_OBJWIN = 7
  };

  void CheckVerticalCounterIRQ();
  void OnScanlineComplete(int cycles_late);
  void OnHblankComplete(int cycles_late);
  void OnVblankScanlineComplete(int cycles_late);
  void OnVblankHblankComplete(int cycles_late);

  void RenderScanline();
  void RenderLayerText(int id);
  void RenderLayerAffine(int id);
  void RenderLayerBitmap1();
  void RenderLayerBitmap2();
  void RenderLayerBitmap3();
  void RenderLayerOAM(bool bitmap_mode, int line);
  void RenderWindow(int id);

  static auto ConvertColor(u16 color) -> u32;

  template<bool window, bool blending>
  void ComposeScanlineTmpl(int bg_min, int bg_max);
  void ComposeScanline(int bg_min, int bg_max);
  void Blend(u16& target1, u16 target2, BlendControl::Effect sfx);

  #include "helper.inl"

  u8 pram[0x00400];
  u8 oam [0x00400];
  u8 vram[0x18000];

  Scheduler& scheduler;
  IRQ& irq;
  DMA& dma;
  std::shared_ptr<Config> config;

  u16 buffer_bg[4][240];

  bool line_contains_alpha_obj;

  struct ObjectPixel {
    u16 color;
    u8  priority;
    unsigned alpha  : 1;
    unsigned window : 1;
  } buffer_obj[240];

  bool buffer_win[2][240];
  bool window_scanline_enable[2];

  u32 output[240*160];

  static constexpr u16 s_color_transparent = 0x8000;
  static const int s_obj_size[4][4][2];
};

} // namespace nba::core
