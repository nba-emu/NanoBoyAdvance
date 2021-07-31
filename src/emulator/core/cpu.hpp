/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <common/log.hpp>
#include <common/punning.hpp>
#include <emulator/cartridge/backup/backup.hpp>
#include <emulator/cartridge/gpio/gpio.hpp>
#include <emulator/cartridge/game_pak.hpp>
#include <emulator/config/config.hpp>
#include <memory>
#include <type_traits>

#include "arm/arm7tdmi.hpp"
#include "hw/apu/apu.hpp"
#include "hw/ppu/ppu.hpp"
#include "hw/dma.hpp"
#include "hw/interrupt.hpp"
#include "hw/serial.hpp"
#include "hw/timer.hpp"
#include "scheduler.hpp"

namespace nba::core {

struct CPU final : private arm::ARM7TDMI, private arm::MemoryBase {
  using Access = arm::MemoryBase::Access;

  CPU(std::shared_ptr<Config> config);

  void Reset();
  void RunFor(int cycles);

  enum class HaltControl {
    RUN,
    STOP,
    HALT
  };

  std::shared_ptr<Config> config;

  struct SystemMemory {
    u8 bios[0x04000];
    u8 wram[0x40000];
    u8 iram[0x08000];
    u32 bios_latch = 0;
  } memory;

  GamePak game_pak;

  struct MMIO {
    u16 keyinput = 0x3FF;
    u16 rcnt_hack = 0;
    u8 postflg = 0;
    HaltControl haltcnt = HaltControl::RUN;

    struct WaitstateControl {
      int sram = 0;
      int ws0_n = 0;
      int ws0_s = 0;
      int ws1_n = 0;
      int ws1_s = 0;
      int ws2_n = 0;
      int ws2_s = 0;
      int phi = 0;
      int prefetch = 0;
      int cgb = 0;
    } waitcnt;

    struct KeyControl {
      uint16_t input_mask = 0;
      bool interrupt = false;
      bool and_mode = false;
    } keycnt;
  } mmio;

  Scheduler scheduler;
  IRQ irq;
  DMA dma;
  APU apu;
  PPU ppu;
  Timer timer;
  SerialBus serial_bus;

private:
  auto ReadMMIO(u32 address) -> u8;
  void WriteMMIO(u32 address, u8 value);
  void WriteMMIO16(u32 address, u16 value);
  auto ReadBIOS(u32 address) -> u32;
  auto ReadUnused(u32 address) -> u32;

  template<typename T>
  auto Read(u32 address, Access access) -> T;

  template<typename T>
  void Write(u32 address, T value, Access access);

  auto ReadByte(u32 address, Access access) -> u8  final {
    return Read<u8>(address, access);
  }

  auto ReadHalf(u32 address, Access access) -> u16 final {
    return Read<u16>(address, access);
  }

  auto ReadWord(u32 address, Access access) -> u32 final {
    return Read<u32>(address, access);
  }

  void WriteByte(u32 address, u8  value, Access access) final {
    Write<u8>(address, value, access);
  }

  void WriteHalf(u32 address, u16 value, Access access) final {
    Write<u16>(address, value, access);
  }

  void WriteWord(u32 address, u32 value, Access access) final {
    Write<u32>(address, value, access);
  }

  void Idle() {
    PrefetchStepRAM(1);
  }

  void ALWAYS_INLINE Tick(int cycles) noexcept {
    openbus_from_dma = false;
    
    if (unlikely(dma.IsRunning() && !bus_is_controlled_by_dma)) {
      bus_is_controlled_by_dma = true;
      dma.Run();
      bus_is_controlled_by_dma = false;
      openbus_from_dma = true;
    }

    scheduler.AddCycles(cycles);

    if (prefetch.active && !bus_is_controlled_by_dma) {
      prefetch.countdown -= cycles;

      if (prefetch.countdown <= 0) {
        prefetch.count++;
        prefetch.active = false;
      }
    }
  }

  void ALWAYS_INLINE PrefetchStepRAM(int cycles) noexcept {
    if (the_pain) {
      return;
    }

    // TODO: bypass prefetch RAM step during DMA?
    if (unlikely(!mmio.waitcnt.prefetch)) {
      Tick(cycles);
      return;
    }

    auto thumb = state.cpsr.f.thumb;
    auto r15 = state.r15;

    /* During any execute cycle except for the fetch cycle, 
     * r15 will be three instruction ahead instead of two.
     */
    if (!code) {
      r15 -= thumb ? 2 : 4;
    }

    if (!prefetch.active && prefetch.rom_code_access && prefetch.count < prefetch.capacity) {
      if (prefetch.count == 0) {
        if (thumb) {
          prefetch.opcode_width = 2;
          prefetch.capacity = 8;
          prefetch.duty = cycles16[int(Access::Sequential)][r15 >> 24];
        } else {
          prefetch.opcode_width = 4;
          prefetch.capacity = 4;
          prefetch.duty = cycles32[int(Access::Sequential)][r15 >> 24];
        }
        prefetch.last_address = r15 + prefetch.opcode_width;
        prefetch.head_address = prefetch.last_address;
      } else {
        prefetch.last_address += prefetch.opcode_width;
      }

      prefetch.countdown = prefetch.duty;
      prefetch.active = true;
    }

    Tick(cycles);
  }

  void ALWAYS_INLINE PrefetchStepROM(u32 address, int cycles) noexcept {
    if (the_pain) {
      return;
    }

    // TODO: bypass prefetch ROM step during DMA?
    if (unlikely(!mmio.waitcnt.prefetch)) {
      Tick(cycles);
      return;
    }

    prefetch.rom_code_access = code;

    if (prefetch.active) {
      if (code && address == prefetch.last_address) {
        // Complete the load and consume the fetched (half)word right away.
        Tick(prefetch.countdown);
        prefetch.count--;
        return;
      }

      prefetch.active = false;
    }

    if (code && prefetch.count != 0) {
      if (address == prefetch.head_address) {
        prefetch.count--;
        prefetch.head_address += prefetch.opcode_width;
        PrefetchStepRAM(1);
        return;
      } else {
        prefetch.count = 0;
      }
    }

    Tick(cycles);
  }

  void UpdateMemoryDelayTable();
  void CheckKeypadInterrupt();
  void OnKeyPress();
  void MP2KSearchSoundMainRAM();
  void MP2KOnSoundMainRAMCalled();

  u32 mp2k_soundmain_address;

  /* GamePak prefetch buffer state. */
  struct Prefetch {
    bool active = false;
    bool rom_code_access = false;
    u32 head_address;
    u32 last_address;
    int count = 0;
    int capacity = 8;
    int opcode_width = 4;
    int countdown;
    int duty;
  } prefetch;

  bool bus_is_controlled_by_dma;
  bool openbus_from_dma;

  int cycles16[2][256] {
    { 1, 1, 3, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 3, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
  };

  int cycles32[2][256] {
    { 1, 1, 6, 1, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 6, 1, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1 }
  };

  static constexpr int s_ws_nseq[4] = { 4, 3, 2, 8 }; /* Non-sequential SRAM/WS0/WS1/WS2 */
  static constexpr int s_ws_seq0[2] = { 2, 1 };       /* Sequential WS0 */
  static constexpr int s_ws_seq1[2] = { 4, 1 };       /* Sequential WS1 */
  static constexpr int s_ws_seq2[2] = { 8, 1 };       /* Sequential WS2 */
};

#include "cpu-memory.inl"

} // namespace nba::core
