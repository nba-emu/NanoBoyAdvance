/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <atomic>
#include <condition_variable>
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

  void Reset();

  void LoadState(SaveState const& state);
  void CopyState(SaveState& state);

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
  auto ALWAYS_INLINE ReadVRAM_BG(u32 address) noexcept -> T {
    return read<T>(vram, address);
  }

  template<typename T>
  auto ALWAYS_INLINE ReadVRAM_OBJ(u32 address, u32 boundary) noexcept -> T {
    if (address >= 0x18000) {
      address &= ~0x8000;

      if (address < boundary) {
        // TODO: check if this should actually return open bus.
        return 0;
      }
    }

    return read<T>(vram, address);
  }

  template<typename T>
  auto ALWAYS_INLINE WriteVRAM_BG(u32 address, T value) noexcept {
    if constexpr (std::is_same_v<T, u8>) {
      write<u16>(vram, address & ~1, value * 0x0101);
    } else {
      write<T>(vram, address, value);
    }
  }

  template<typename T>
  auto ALWAYS_INLINE WriteVRAM_OBJ(u32 address, T value, u32 boundary) noexcept {
    if constexpr (!std::is_same_v<T, u8>) {
      if (address >= 0x18000) {
        address &= ~0x8000;

        if (address < boundary) {
          return;
        }
      }

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

  void ALWAYS_INLINE Sync() {
    // TODO: remove this redundant method
    SyncLineRender();
  }

  auto ALWAYS_INLINE GetSpriteVRAMBoundary() noexcept -> u32 {
    return mmio.dispcnt.mode >= 3 ? 0x14000 : 0x10000;
  }

  bool ALWAYS_INLINE DidAccessPRAM() const {
    return pram_access;
  }

  bool ALWAYS_INLINE DidAccessVRAM_BG() const  {
    return vram_bg_access;
  }

  bool ALWAYS_INLINE DidAccessVRAM_OBJ() const {
    return false;
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

  static constexpr int kLastRenderCycle = 1008;

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
  void UpdateVideoTransferDMA();

  void OnScanlineComplete(int cycles_late);
  void OnHblankIRQTest(int cycles_late);
  void OnHblankComplete(int cycles_late);
  void OnVblankScanlineComplete(int cycles_late);
  void OnVblankHblankIRQTest(int cycles_late);
  void OnVblankHblankComplete(int cycles_late);

  // TODO: think of less terrible names:
  void InitLineRender();
  void SyncLineRender();
  void InitBG(int id);
  void SyncBG(int id, int cycles);
  void FetchMapMode0(int id);
  void FetchTileMode04BPP(int id);
  void FetchTileMode08BPP(int id);
  void RenderBGMode0(int id, int cycles);
  void RenderBGMode2(int id, int cycles);
  void RenderBGMode3(int cycles);
  void RenderBGMode4(int cycles);
  void RenderBGMode5(int cycles);
  void InitCompose();
  void SyncCompose(int cycles);
  auto Blend(u16 color_a, u16 color_b) -> u16;
  auto Brighten(u16 color) -> u16;
  auto Darken(u16 color) -> u16;
  void RenderLayerOAM(bool bitmap_mode, int line);
  void UpdateWindows();

  #include "helper.inl"

  u8 pram[0x00400];
  u8 oam [0x00400];
  u8 vram[0x18000];

  int dispcnt_mode;
  u64 last_sync_point;

  struct Latch {
    BackgroundControl bgcnt[4];
    u16 bghofs[4];
  } latch;

  struct BG {
    bool engaged;

    int x;
    int hcounter;

    // TODO: share address field of Text and Affine structs?
    struct Text {
      int grid_x;
      int palette;
      bool flip_x;
      bool full_palette;
      u32 address;
    } text;

    struct Affine {
      s32 x;
      s32 y;

      // Mode1, Mode2 text-based affine modes:
      bool fetching;
      u32 address;
    } affine;

    u32 buffer[256];
  } bg[4];

  struct Compose {
    bool engaged;
    int x;
    int hcounter;
  } compose;

  struct ObjectPixel {
    u32 color;
    u8  priority;
    unsigned alpha  : 1;
    unsigned window : 1;
    unsigned mosaic : 1;
  } buffer_obj[240];

  bool window_flag_h[2];
  bool window_flag_v[2];

  bool pram_access;
  bool vram_bg_access;

  Scheduler& scheduler;
  IRQ& irq;
  DMA& dma;
  std::shared_ptr<Config> config;

  u32 output[2][240 * 160];
  int frame;

  bool dma3_video_transfer_running;
};

} // namespace nba::core
