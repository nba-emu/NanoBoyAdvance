/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <array>
#include <nba/rom/rom.hpp>
#include <nba/integer.hpp>
#include <vector>

#include "emulator/core/hw/apu/apu.hpp"
#include "emulator/core/hw/ppu/ppu.hpp"
#include "emulator/core/hw/dma.hpp"
#include "emulator/core/hw/interrupt.hpp"
#include "emulator/core/hw/keypad.hpp"
#include "emulator/core/hw/timer.hpp"

namespace nba::core {

namespace {
struct ARM7TDMI;
} // namespace nba::core::arm

struct Bus {
  enum class Access {
    Nonsequential = 0,
    Sequential  = 1
  };

  void Reset();
  void Attach(std::vector<u8> const& bios);
  void Attach(ROM&& rom);

  auto ReadByte(u32 address, Access access) ->  u8;
  auto ReadHalf(u32 address, Access access) -> u16;
  auto ReadWord(u32 address, Access access) -> u32;

  void WriteByte(u32 address, u8  value, Access access);
  void WriteHalf(u32 address, u16 value, Access access);
  void WriteWord(u32 address, u32 value, Access access);

  void Idle();

//private:
  Scheduler& scheduler;

  struct Memory {
    std::array<u8, 0x04000> bios;
    std::array<u8, 0x40000> wram;
    std::array<u8, 0x08000> iram;
    struct Latch {
      u32 bios = 0;
    } latch;
    ROM rom;
  } memory;

  struct Hardware {
    arm::ARM7TDMI& cpu;
    IRQ& irq;
    DMA& dma;
    APU& apu;
    PPU& ppu;
    Timer& timer;
    KeyPad& keypad;
    Bus* bus = nullptr;

    struct WaitstateControl {
      int sram = 0;
      int ws0[2] { 0, 0 };
      int ws1[2] { 0, 0 };
      int ws2[2] { 0, 0 };
      int phi = 0;
      bool prefetch = false;
      bool cgb = false;
    } waitcnt;

    enum class HaltControl {
      Run,
      Stop,
      Halt
    } haltcnt = HaltControl::Run;

    u8 rcnt[2] { 0, 0 };
    u8 postflg = 0;

    auto ReadByte(u32 address) ->  u8;
    auto ReadHalf(u32 address) -> u16;
    auto ReadWord(u32 address) -> u32;

    void WriteByte(u32 address,  u8 value);
    void WriteHalf(u32 address, u16 value);
    void WriteWord(u32 address, u32 value);
  } hw;

  struct Prefetch {
    bool active = false;
    u32 head_address;
    u32 last_address;
    int count = 0;
    int capacity = 8;
    int opcode_width = 4;
    int countdown;
    int duty;
  } prefetch;

  struct DMA {
    bool active = false;
    bool openbus = false;
  } dma;

  template<typename T>
  auto Read(u32 address, Access access) -> T;
  
  template<typename T>
  void Write(u32 address, Access access, T value);

  template<typename T>
  auto Align(u32 address) -> u32 {
    return address & ~(sizeof(T) - 1);
  }

  auto ReadBIOS(u32 address) -> u32;
  auto ReadOpenBus(u32 address) -> u32;

  void Prefetch(u32 address, int cycles);
  void StopPrefetch();
  void Step(int cycles);
  void UpdateWaitStateTable();
 
  int wait16[2][16] {
    { 1, 1, 3, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 3, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
  };

  int wait32[2][16] {
    { 1, 1, 6, 1, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 6, 1, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1 }
  };

public:
  Bus(Scheduler& scheduler, Hardware&& hw);

  auto GetHostAddress(u32 address, size_t size) -> u8*;

  template<typename T>
  auto GetHostAddress(u32 address, size_t count = 1) -> T* {
    return (T*)GetHostAddress(address, sizeof(T) * count);
  }
};

} // namespace nba::core
