/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <atomic>
#include <functional>
#include <nba/common/compiler.hpp>
#include <nba/common/punning.hpp>
#include <nba/config.hpp>
#include <nba/integer.hpp>
#include <nba/save_state.hpp>
#include <thread>
#include <type_traits>

#include "hw/ppu/registers.hpp"
#include "hw/dma/dma.hpp"
#include "hw/irq/irq.hpp"
#include "scheduler.hpp"

namespace nba::core {

struct PPU {
  PPU(
    Scheduler& scheduler,
    IRQ& irq,
    DMA& dma,
    std::shared_ptr<Config> config
  );

 ~PPU(); 

  void Reset();

  void LoadState(SaveState const& state);
  void CopyState(SaveState& state);

  template<typename T>
  auto ALWAYS_INLINE ReadPRAM(u32 address) noexcept -> T {
    return read<T>(pram, address & 0x3FF);
  }

  template<typename T>
  void ALWAYS_INLINE WritePRAM(u32 address, T value) noexcept {
    WaitForRenderThread();

    if constexpr (std::is_same_v<T, u8>) {
      write<u16>(pram, address & 0x3FE, value * 0x0101);

      if (!mmio.dispstat.vblank_flag) {
        // TODO: optimise this
        write<u16>(pram_draw, address & 0x3FE, value * 0x0101);    
      }
    } else {
      write<T>(pram, address & 0x3FF, value);

      if (!mmio.dispstat.vblank_flag) {
        write<T>(pram_draw, address & 0x3FF, value);    
      }
    }
  }

  template<typename T>
  auto ALWAYS_INLINE ReadVRAM(u32 address) noexcept -> T {
    address &= 0x1FFFF;

    if (address >= 0x18000) {
      if ((address & 0x4000) == 0 && mmio.dispcnt.mode >= 3) {
        // TODO: check if this should actually return open bus.
        return 0;
      }
      address &= ~0x8000;
    }

    return read<T>(vram, address);
  }

  template<typename T>
  void ALWAYS_INLINE WriteVRAM(u32 address, T value) noexcept {
    WaitForRenderThread();

    address &= 0x1FFFF;

    if (address >= 0x18000) {
      if ((address & 0x4000) == 0 && mmio.dispcnt.mode >= 3) {
        return;
      }
      address &= ~0x8000;
    }

    if (std::is_same_v<T, u8>) {
      auto limit = mmio.dispcnt.mode >= 3 ? 0x14000 : 0x10000;

      if (address < limit) {
        write<u16>(vram, address & ~1, value * 0x0101);

        if (!mmio.dispstat.vblank_flag) {
          // TODO: optimise this.
          write<u16>(vram_draw, address & ~1, value * 0x0101);
        } else {
          vram_dirty_range_lo = std::min(vram_dirty_range_lo, (int)address);
          vram_dirty_range_hi = std::max(vram_dirty_range_hi, (int)(address + sizeof(T)));
        }
      }
    } else {
      write<T>(vram, address, value);

      if (!mmio.dispstat.vblank_flag) {
        write<T>(vram_draw, address, value);    
      } else {
        vram_dirty_range_lo = std::min(vram_dirty_range_lo, (int)address);
        vram_dirty_range_hi = std::max(vram_dirty_range_hi, (int)(address + sizeof(T)));
      }
    }
  }

  template<typename T>
  auto ALWAYS_INLINE ReadOAM(u32 address) noexcept -> T {
    return read<T>(oam, address & 0x3FF);
  }

  template<typename T>
  void ALWAYS_INLINE WriteOAM(u32 address, T value) noexcept {
    WaitForRenderThread();

    if constexpr (!std::is_same_v<T, u8>) {
      write<T>(oam, address & 0x3FF, value);

      if (!mmio.dispstat.vblank_flag) {
        write<T>(oam_draw, address & 0x3FF, value);    
      }
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

    bool enable_bg[2][4];
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

  void LatchEnabledBGs();
  void LatchBGXYWrites();
  void CheckVerticalCounterIRQ();

  void OnScanlineComplete(int cycles_late);
  void OnHblankIRQTest(int cycles_late);
  void OnHblankComplete(int cycles_late);
  void OnVblankScanlineComplete(int cycles_late);
  void OnVblankHblankIRQTest(int cycles_late);
  void OnVblankHblankComplete(int cycles_late);

  void RenderScanline(int vcount);
  void RenderLayerText(int vcount, int id);
  void RenderLayerAffine(int vcount, int id);
  void RenderLayerBitmap1(int vcount);
  void RenderLayerBitmap2(int vcount);
  void RenderLayerBitmap3(int vcount);
  void RenderLayerOAM(bool bitmap_mode, int vcount, int line);
  void RenderWindow(int vcount, int id);

  static auto ConvertColor(u16 color) -> u32;

  template<bool window, bool blending>
  void ComposeScanlineTmpl(int vcount, int bg_min, int bg_max);
  void ComposeScanline(int vcount, int bg_min, int bg_max);
  void Blend(int vcount, u16& target1, u16 target2, BlendControl::Effect sfx);

  void SetupRenderThread();
  void StopRenderThread();
  void SubmitScanline();
  void ScheduleSubmitScanline();

  void WaitForRenderThread() {
    if (mmio.vcount < 160 && render_thread_vcount < 160) {
      while (render_thread_vcount <= render_thread_vcount_max) ;
    }
  }

  #include "helper.inl"

  u8 pram[0x00400];
  u8 oam [0x00400];
  u8 vram[0x18000];

  u8 pram_draw[0x00400];
  u8  oam_draw[0x00400];
  u8 vram_draw[0x18000];

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
    unsigned mosaic : 1;
  } buffer_obj[240];

  bool buffer_win[2][240];
  bool window_scanline_enable[2];

  u32 output[2][240 * 160];
  int frame;

  std::thread render_thread;
  std::atomic_int render_thread_vcount;
  std::atomic_int render_thread_vcount_max;
  std::atomic_bool render_thread_running = false;
  MMIO mmio_copy[228];
  int vram_dirty_range_lo;
  int vram_dirty_range_hi;

  static constexpr u16 s_color_transparent = 0x8000;
  static const int s_obj_size[4][4][2];
};

} // namespace nba::core
