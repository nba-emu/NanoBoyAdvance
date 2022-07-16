/*
 * Copyright (C) 2022 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <array>
#include <nba/integer.hpp>

namespace nba {

// TODO: optimise for size?

struct SaveState {
  static constexpr u32 kMagicNumber = 0x5353424E; // NBSS

  u32 magic;
  u32 version;
  u64 timestamp;

  struct ARM {
    // TODO: come up with a cleaner structure?
    struct {
      u32 gpr[16];
      u32 bank[6][7];
      u32 cpsr;
      u32 spsr[6];
    } regs;

    struct Pipeline {
      u8 access;
      u32 opcode[2];
    } pipe;

    bool irq_line;

    // TODO:
    //bool ldm_usermode_conflict;
    //bool cpu_mode_is_invalid;
    //bool latch_irq_disable;
  } arm;

  struct Bus {
    struct Memory {
      std::array<u8, 0x40000> wram;
      std::array<u8, 0x8000> iram;
      struct Latch {
        u32 bios;
      } latch;
    } memory;

    struct IO {
      struct WaitstateControl {
        u8 sram;
        u8 ws0[2];
        u8 ws1[2];
        u8 ws2[2];
        u8 phi;
        bool prefetch;
      } waitcnt;

      u8 haltcnt;
      u8 rcnt[2];
      u8 postflg;
    } io;

    struct Prefetch {
      bool active;
      u32 head_address;
      u32 last_address;
      u8 count;
      u8 countdown;
    } prefetch;

    struct DMA {
      bool active;
      bool openbus;
    } dma;
  } bus;

  // TODO: keep track of IRQ delay:
  struct IRQ {
    u8  reg_ime;
    u16 reg_ie;
    u16 reg_if;
  } irq;

  struct PPU {
    struct IO {

    } io;

    u8 pram[0x00400];
    u8 oam [0x00400];
    u8 vram[0x18000];
  } ppu;
};

} // namespace nba
