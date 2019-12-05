/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#pragma once

#include <memory>

#include "arm/arm7.hpp"
#include "backup/backup.hpp"
#include "dma.hpp"
#include "timer.hpp"
#include "scheduler.hpp"
#include "apu/apu.hpp"
#include "ppu/ppu.hpp"
#include "../config/config.hpp"

namespace GameBoyAdvance {

class CPU : private ARM::Interface {
public:
  CPU(std::shared_ptr<Config> config);

  void Reset();
  void RunFor(int cycles);
  
  /* TODO: provide way to read CPU memory without consuming cycles. */
  
  auto ReadByte(std::uint32_t address, ARM::AccessType type) -> std::uint8_t  final;
  auto ReadHalf(std::uint32_t address, ARM::AccessType type) -> std::uint16_t final;
  auto ReadWord(std::uint32_t address, ARM::AccessType type) -> std::uint32_t final;
  void WriteByte(std::uint32_t address, std::uint8_t value, ARM::AccessType type)  final;
  void WriteHalf(std::uint32_t address, std::uint16_t value, ARM::AccessType type) final;
  void WriteWord(std::uint32_t address, std::uint32_t value, ARM::AccessType type) final;
  
  enum MemoryRegion {
    REGION_BIOS = 0,
    REGION_EWRAM = 2,
    REGION_IWRAM = 3,
    REGION_MMIO = 4,
    REGION_PRAM = 5,
    REGION_VRAM = 6,
    REGION_OAM = 7,
    REGION_ROM_W0_L = 8,
    REGION_ROM_W0_H = 9,
    REGION_ROM_W1_L = 0xA,
    REGION_ROM_W1_H = 0xB,
    REGION_ROM_W2_L = 0xC,
    REGION_ROM_W2_H = 0xD,
    REGION_SRAM_1 = 0xE,
    REGION_SRAM_2 = 0xF
  };
  
  enum class HaltControl {
    RUN,
    STOP,
    HALT
  };

  enum Interrupt {
    INT_VBLANK  = 1 << 0,
    INT_HBLANK  = 1 << 1,
    INT_VCOUNT  = 1 << 2,
    INT_TIMER0  = 1 << 3,
    INT_TIMER1  = 1 << 4,
    INT_TIMER2  = 1 << 5,
    INT_TIMER3  = 1 << 6,
    INT_SERIAL  = 1 << 7,
    INT_DMA0    = 1 << 8,
    INT_DMA1    = 1 << 9,
    INT_DMA2    = 1 << 10,
    INT_DMA3    = 1 << 11,
    INT_KEYPAD  = 1 << 12,
    INT_GAMEPAK = 1 << 13
  };
  
  std::shared_ptr<Config> config;

  struct SystemMemory {
    std::uint8_t bios[0x04000];
    std::uint8_t wram[0x40000];
    std::uint8_t iram[0x08000];
    std::uint8_t pram[0x00400];
    std::uint8_t oam [0x00400];
    std::uint8_t vram[0x18000];

    struct ROM {
      std::shared_ptr<uint8_t[]> data;
      std::shared_ptr<Backup> backup;
      size_t size;
      std::uint32_t mask = 0x1FFFFFF;
    } rom;

    /* Last opcode fetched from BIOS memory. */
    std::uint32_t bios_opcode;
  } memory;

  struct MMIO {
    std::uint16_t keyinput;

    std::uint16_t irq_ie;
    std::uint16_t irq_if;
    int irq_ime;

    HaltControl haltcnt;

    struct WaitstateControl {
      int sram;
      int ws0_n;
      int ws0_s;
      int ws1_n;
      int ws1_s;
      int ws2_n;
      int ws2_s;
      int phi;
      int prefetch;
      int cgb;
    } waitcnt;
  } mmio; 
  
  Scheduler scheduler;
  
  APU apu;
  PPU ppu;
  DMA dma;
  Timer timer;
  ARM::ARM7TDMI cpu;
  
private:
  
  template <typename T>
  auto Read(void* buffer, std::uint32_t address) -> T {
    return *(T*)(&((std::uint8_t*)buffer)[address]);
  }
  
  template <typename T>
  void Write(void* buffer, std::uint32_t address, T value) {
    *(T*)(&((std::uint8_t*)buffer)[address]) = value;
  }
  
  auto ReadMMIO (std::uint32_t address) -> std::uint8_t;
  void WriteMMIO(std::uint32_t address, std::uint8_t value);
  auto ReadBIOS(std::uint32_t address) -> std::uint32_t;
  auto ReadUnused(std::uint32_t address) -> std::uint32_t;
  
  void SWI(std::uint32_t call_id) final { }
  void Tick(int cycles) final;
  void Idle() final;
  void PrefetchStep(std::uint32_t address, int cycles);
  
  void UpdateCycleLUT();
  
  /* GamePak prefetch buffer state. */
  struct Prefetch {
    bool active;
    std::uint32_t address[8];
    std::uint32_t last_address;
    int rd_pos;
    int wr_pos;
    int count;
    int countdown;
  } prefetch;
  
  /* Last ROM address that was accessed. Used for GamePak prefetch. */
  std::uint32_t last_rom_address;
  
  cycle_t ticks_cpu_left = 0;
  cycle_t ticks_to_event = 0;

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

#include "memory/memory.inl"
  
}
