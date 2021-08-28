/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <array>
#include <nba/integer.hpp>

#include "emulator/cartridge/game_pak.hpp"
#include "emulator/core/arm/memory.hpp"
#include "emulator/core/hw/apu/apu.hpp"
#include "emulator/core/hw/ppu/ppu.hpp"
#include "emulator/core/hw/dma.hpp"
#include "emulator/core/hw/interrupt.hpp"
#include "emulator/core/hw/serial.hpp"
#include "emulator/core/hw/timer.hpp"

namespace nba::core {

struct CPU;

struct Bus : arm::MemoryBase {
  void Reset();

  auto ReadByte(u32 address, Access access) ->  u8 override;
  auto ReadHalf(u32 address, Access access) -> u16 override;
  auto ReadWord(u32 address, Access access) -> u32 override;

  void WriteByte(u32 address, u8  value, Access access) override;
  void WriteHalf(u32 address, u16 value, Access access) override;
  void WriteWord(u32 address, u32 value, Access access) override;

  void Idle() override;

//private:
  Scheduler& scheduler;

  struct Memory {
    std::array<u8, 0x04000> bios;
    std::array<u8, 0x40000> wram;
    std::array<u8, 0x08000> iram;
    struct Latch {
      u32 bios = 0;
    } latch;
    GamePak game_pak;
  } memory;

  struct Hardware {
    CPU& cpu;
    IRQ& irq;
    DMA& dma;
    APU& apu;
    PPU& ppu;
    Timer& timer;
    SerialBus& serial_bus;
    Bus* bus = nullptr;

    auto ReadByte(u32 address) ->  u8;
    auto ReadHalf(u32 address) -> u16;
    auto ReadWord(u32 address) -> u32;

    void WriteByte(u32 address,  u8 value);
    void WriteHalf(u32 address, u16 value);
    void WriteWord(u32 address, u32 value);
  } hw;

  bool inside_dma;

  template<typename T> auto Read (u32 address, Access access) -> T;
  template<typename T> void Write(u32 address, Access access, T value);

  void PrefetchStepROM(u32 address, int cycles);
  void PrefetchStepRAM(int cycles);
  void UpdateWaitStateTable();
  auto ReadBIOS(u32 address) -> u32;
  auto ReadOpenBus(u32 address) -> u32;

public:
  Bus(Scheduler& scheduler, Hardware&& hw);
};

} // namespace nba::core
