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

#include <arm/arm7tdmi/arm7tdmi.hpp>
#include <memory>

#include "dma.hpp"
#include "timer.hpp"
#include "scheduler.hpp"
#include "apu/apu.hpp"
#include "ppu/ppu.hpp"
#include "../config.hpp"

namespace GameBoyAdvance {

class CPU : private ARM::Interface {
public:
  CPU(std::shared_ptr<Config> config);

  void Reset();
  
  void RunFor(int cycles);
  
  /* TODO: provide way to read CPU memory without consuming cycles. */
  std::uint8_t  ReadByte(std::uint32_t address, ARM::AccessType type) final;
  std::uint16_t ReadHalf(std::uint32_t address, ARM::AccessType type) final;
  std::uint32_t ReadWord(std::uint32_t address, ARM::AccessType type) final;
  void WriteByte(std::uint32_t address, std::uint8_t value, ARM::AccessType type) final;
  void WriteHalf(std::uint32_t address, std::uint16_t value, ARM::AccessType type) final;
  void WriteWord(std::uint32_t address, std::uint32_t value, ARM::AccessType type) final;
  
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
      size_t size;
    } rom;

    /* Last opcode fetched from BIOS memory. */
    std::uint32_t bios_opcode;
  } memory;

  struct MMIO {
    std::uint16_t keyinput;

    /* TODO: make this accessible without passing
     *       the complete CPU object. 
     */
    std::uint16_t irq_ie;
    std::uint16_t irq_if;
    std::uint16_t irq_ime;

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
  
  auto ReadMMIO (std::uint32_t address) -> std::uint8_t;
  void WriteMMIO(std::uint32_t address, std::uint8_t value);
  std::uint32_t ReadBIOS(std::uint32_t address);
  
  void SWI(std::uint32_t call_id) final { }
  void Tick(int cycles) final;
  void Idle() final;
  void RunPrefetch(std::uint32_t address, int cycles);
  
  void UpdateCycleLUT();
  
  struct Prefetch {
    bool active;
    std::uint32_t address;
    int countdown;
  } prefetch;
  
  cycle_t ticks_cpu_left = 0;
  cycle_t ticks_to_event = 0;

  int cycles16[2][16] {
    { 1, 1, 3, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 3, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
  };
  
  int cycles32[2][16] {
    { 1, 1, 6, 1, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1 },
    { 1, 1, 6, 1, 1, 2, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1 }
  };
  
  static constexpr int s_ws_nseq[4] = { 4, 3, 2, 8 }; /* Non-sequential SRAM/WS0/WS1/WS2 */
  static constexpr int s_ws_seq0[2] = { 2, 1 };       /* Sequential WS0 */
  static constexpr int s_ws_seq1[2] = { 4, 1 };       /* Sequential WS1 */
  static constexpr int s_ws_seq2[2] = { 8, 1 };       /* Sequential WS2 */
};

#include "cpu-memory.inl"
  
}
