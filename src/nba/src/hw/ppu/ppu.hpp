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
    DrawBackground();
    DrawSprite();
    DrawMerge();
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

    u8 buffer[240][4];
  } bg;

  void InitBackground();
  void DrawBackground();
  template<int mode> void DrawBackgroundImpl(int cycles);

  enum class OAMFetchState {
    Attribute01,
    Attribute2,
    PA,
    PB,
    PC,
    PD
  };

  struct Sprite {
    u64 timestamp_init = 0;
    u64 timestamp_last_sync = 0;
    u64 timestamp_vram_access = ~0ULL;
    u64 timestamp_oam_access = ~0ULL;
    uint cycle;
    uint vcount;
    uint index;

    OAMFetchState oam_fetch_state;
    int oam_access_wait;
    bool first_vram_access_cycle;
    bool drawing;

    struct {
      s32 x;
      s32 y;
      int width;
      int height;
      int half_width;
      int half_height;
      int mode;
      bool affine;

      int local_x;
      int local_y;

      int transform_id;
      s16 transform[4];

      int tile_number;
      int priority;
      int palette;
      bool flip_h;
      bool flip_v;
      bool is_256;
    } state[2];

    int state_rd;
    int state_wr;

    u32 buffer[2][240];
    u32* buffer_rd;
    u32* buffer_wr;
  } sprite;

  void InitSprite();
  void DrawSprite();
  void DrawSpriteImpl(int cycles);
  void DrawSpriteFetchOAM(uint cycle);
  void DrawSpriteFetchVRAM(uint cycle);
  void StupidSpriteEventHandler(int cycles);

  struct Merge {
    u64 timestamp_init = 0;
    u64 timestamp_last_sync = 0;
    uint cycle;
  } merge;

  void InitMerge();
  void DrawMerge();
  void DrawMergeImpl(int cycles);

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
