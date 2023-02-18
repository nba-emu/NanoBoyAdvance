/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <functional>
#include <nba/common/compiler.hpp>
#include <nba/common/punning.hpp>
#include <nba/config.hpp>
#include <nba/integer.hpp>
#include <nba/save_state.hpp>
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

  auto ALWAYS_INLINE GetSpriteVRAMBoundary() noexcept -> u32 {
    return mmio.dispcnt.mode >= 3 ? 0x14000 : 0x10000;
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
        // @todo: check if this should actually return open bus.
        return 0;
      }
    }

    return read<T>(vram, address);
  }

  template<typename T>
  void ALWAYS_INLINE WriteVRAM(u32 address, T value) noexcept {
    const u32 boundary = GetSpriteVRAMBoundary();

    address &= 0x1FFFF;

    if (address >= boundary) {
      WriteVRAM_OBJ<T>(address, value, boundary);
    } else {
      WriteVRAM_BG<T>(address, value);
    }
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

  bool ALWAYS_INLINE DidAccessPRAM() noexcept {
    return scheduler.GetTimestampNow() == merge.timestamp_pram_access + 1U;
  }

  bool ALWAYS_INLINE DidAccessVRAM_BG() noexcept {
    return scheduler.GetTimestampNow() == bg.timestamp_vram_access;
  }

  bool ALWAYS_INLINE DidAccessVRAM_OBJ() noexcept {
    return scheduler.GetTimestampNow() == sprite.timestamp_vram_access + 1U;
  }

  bool ALWAYS_INLINE DidAccessOAM() noexcept {
    return scheduler.GetTimestampNow() == sprite.timestamp_oam_access + 1U;
  }

  void Sync() {
    // @todo: only update the window when it is necessary or else
    // we will have a major performance caveat due to the window being updated 
    // during V-blank and games typically updating graphics during V-blank.
    DrawBackground();
    DrawSprite();
    DrawWindow();
    DrawMerge();
  }

  struct MMIO {
    DisplayControl dispcnt;
    DisplayStatus dispstat;

    u16 greenswap;

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

    bool enable_bg[3][4];
  } mmio;

private:
  static constexpr uint k_bg_cycle_limit = 1006;

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
  void UpdateVideoTransferDMA();

  void OnScanlineComplete(int cycles_late);
  void OnHblankIRQTest(int cycles_late);
  void OnHblankComplete(int cycles_late);
  void OnVblankScanlineComplete(int cycles_late);
  void OnVblankHblankIRQTest(int cycles_late);
  void OnVblankHblankComplete(int cycles_late);

  struct Background {
    u64 timestamp_init = 0;
    u64 timestamp_last_sync = 0;
    u64 timestamp_vram_access = ~0ULL;
    uint cycle;

    struct Text {
      bool fetching;

      struct Tile {
        u32 address;
        uint palette;
        bool flip_x;
      } tile;

      struct PISO {
        u16 data;
        int remaining;
      } piso;
    } text[4];

    struct Affine {
      s32 x;
      s32 y;
      bool out_of_bounds;
      u16 tile_address;
    } affine[2];

    u32 buffer[240][4];

    int mosaic_y;
  } bg;

  void InitBackground();
  void DrawBackground();
  template<int mode> void DrawBackgroundImpl(int cycles);

  struct Sprite {
    u64 timestamp_init = 0;
    u64 timestamp_last_sync = 0;
    u64 timestamp_vram_access = ~0ULL;
    u64 timestamp_oam_access = ~0ULL;
    uint cycle;
    uint vcount;
    int mosaic_y;

    struct {
      uint index;
      int  step;
      int  wait;
      int  pending_wait;
      bool delay_wait;
      int  initial_local_x;
      int  initial_local_y;
      uint matrix_address;
    } oam_fetch;

    bool drawing;

    struct {
      int width;
      int height;
      int mode;
      bool mosaic;
      bool affine;

      int draw_x;
      int remaining_pixels;

      s16 matrix[4];

      uint tile_number;
      uint priority;
      uint palette;
      bool flip_h;
      bool is_256;

      int texture_x;
      int texture_y;
    } drawer_state[2];

    int state_rd;
    int state_wr;

    union Pixel {
      struct {
        u8 color : 8;
        unsigned priority : 2;
        unsigned alpha  : 1;
        unsigned window : 1;
        unsigned mosaic : 1;
      };
      u16 data;
    };

    Pixel buffer[2][240];
    Pixel* buffer_rd;
    Pixel* buffer_wr;

    uint latch_cycle_limit;
  } sprite;

  void InitSprite();
  void DrawSprite();
  void DrawSpriteImpl(int cycles);
  void DrawSpriteFetchOAM(uint cycle);
  void DrawSpriteFetchVRAM(uint cycle);
  void StupidSpriteEventHandler(int cycles);

  struct Window {
    u64 timestamp_last_sync;
    uint cycle;

    bool v_flag[2] {false, false};
    bool h_flag[2] {false, false};

    bool buffer[240][2];
  } window;

  void InitWindow();
  void DrawWindow();

  struct Merge {
    u64 timestamp_init = 0;
    u64 timestamp_last_sync = 0;
    u64 timestamp_pram_access = 0;
    uint cycle;
    uint mosaic_x[2];
    int layers[2];
    bool force_alpha_blend;
    u32 colors[2];
    u16 color_l;
  } merge;

  void InitMerge();
  void DrawMerge();
  void DrawMergeImpl(int cycles);
  
  static auto Blend(u16 color_a, u16 color_b, int eva, int evb) -> u16;
  static auto Brighten(u16 color, int evy) -> u16;
  static auto Darken(u16 color, int evy) -> u16;

  template<typename T>
  auto ALWAYS_INLINE FetchPRAM(uint cycle, uint address) -> T {
    merge.timestamp_pram_access = merge.timestamp_init + cycle;
    return read<T>(pram, address);
  }

  template<typename T>
  auto ALWAYS_INLINE FetchVRAM_BG(uint cycle, uint address) -> T {
    bg.timestamp_vram_access = bg.timestamp_init + cycle;
    return read<T>(vram, address);
  }

  template<typename T>
  auto ALWAYS_INLINE FetchVRAM_OBJ(uint cycle, uint address) -> T {
    sprite.timestamp_vram_access = sprite.timestamp_init + cycle;

    // @todo: verify this edge-case against hardware.
    if(address < GetSpriteVRAMBoundary()) {
      return 0U;
    }
    return read<T>(vram, address);
  }

  template<typename T>
  auto ALWAYS_INLINE FetchOAM(uint cycle, uint address) -> T {
    sprite.timestamp_oam_access = sprite.timestamp_init + cycle;
    return read<T>(oam, address);
  }

  u8 pram[0x00400];
  u8 oam [0x00400];
  u8 vram[0x18000];

  Scheduler& scheduler;
  IRQ& irq;
  DMA& dma;
  std::shared_ptr<Config> config;

  u32 output[2][240 * 160];
  int frame;

  bool dma3_video_transfer_running;

  #include "background.inl"
};

} // namespace nba::core
