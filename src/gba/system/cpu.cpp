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

#include "cpu.hpp"

#include <algorithm>
#include <cstring>
#include <limits>

using namespace GameBoyAdvance;

constexpr int CPU::s_ws_nseq[4];
constexpr int CPU::s_ws_seq0[2];
constexpr int CPU::s_ws_seq1[2];
constexpr int CPU::s_ws_seq2[2];

CPU::CPU(std::shared_ptr<Config> config)
  : config(config)
  , apu(this)
  , ppu(this)
  , dma(this)
  , timer(this)
  , cpu(this)
{
  Reset();
}

void CPU::Reset() {
  // FIXME
  //scheduler.Reset();
  
  scheduler.Add(ppu.event);
  
  /* Clear all memory buffers. */
  std::memset(memory.bios, 0, 0x04000);
  std::memset(memory.wram, 0, 0x40000);
  std::memset(memory.iram, 0, 0x08000);
  std::memset(memory.pram, 0, 0x00400);
  std::memset(memory.oam,  0, 0x00400);
  std::memset(memory.vram, 0, 0x18000);

  /* Reset interrupt control. */
  mmio.irq_ie  = 0;
  mmio.irq_if  = 0;
  mmio.irq_ime = 0;
  
  mmio.keyinput = 0x3FF;
  mmio.haltcnt = HaltControl::RUN;

  /* Reset waitstates. */
  mmio.waitcnt.sram = 0;
  mmio.waitcnt.ws0_n = 0;
  mmio.waitcnt.ws0_s = 0;
  mmio.waitcnt.ws1_n = 0;
  mmio.waitcnt.ws1_s = 0;
  mmio.waitcnt.ws2_n = 0;
  mmio.waitcnt.ws2_s = 0;
  mmio.waitcnt.phi = 0;
  mmio.waitcnt.prefetch = 0;
  mmio.waitcnt.cgb = 0;
  UpdateCycleLUT();
  
  prefetch.active = false;
  prefetch.count = 0;
  
  dma.Reset();
  timer.Reset();
  apu.Reset();
  ppu.Reset();
  cpu.Reset();
  
//  cpu.state.bank[ARM::BANK_SVC][ARM::BANK_R13] = 0x03007FE0; 
//  cpu.state.bank[ARM::BANK_IRQ][ARM::BANK_R13] = 0x03007FA0;
//  cpu.state.reg[13] = 0x03007F00;
//  cpu.state.cpsr.f.mode = ARM::MODE_USR;
//  cpu.state.r15 = 0x08000000;
}

void CPU::Tick(int cycles) {
  timer.Run(cycles);
  ticks_cpu_left -= cycles;
  
  if (prefetch.active) {
    prefetch.countdown -= cycles;
    
    if (prefetch.countdown <= 0) {
      prefetch.count++;
      prefetch.active = false;
    }
  }
}

void CPU::Idle() {
  if (mmio.waitcnt.prefetch) {
    PrefetchStep(0, 1);
  } else {
    Tick(1);
  }
}

void CPU::PrefetchStep(std::uint32_t address, int cycles) {
  #define IS_ROM_REGION(address) ((address) >= 0x08000000 && (address) <= 0x0EFFFFFF) 
  
  auto thumb = cpu.state.cpsr.f.thumb;
  auto capacity = thumb ? 8 : 4;
  
  static std::uint32_t last_rom_addr = 0;
  
  if (prefetch.active) {
    /* If prefetching the desired opcode just complete. */
    if (address == prefetch.address[prefetch.count]) {
      Tick(prefetch.countdown);
      
      /* TODO: this is redundant and slow. */
      prefetch.count--;
      for (int i = 1; i < capacity; i++) {
        prefetch.address[i - 1] = prefetch.address[i];
      }
      
      // blarghz
      last_rom_addr = address;
      return;
    }
    
    if (IS_ROM_REGION(address)) {
      prefetch.active = false;
      //std::printf("prefetch to 0x%08x interrupted by access to 0x%08x.\n", prefetch.address[prefetch.count], address);
    }
  } else if (prefetch.count < capacity && IS_ROM_REGION(cpu.state.r15) && !IS_ROM_REGION(address) && cpu.state.r15 == last_rom_addr) {
    std::uint32_t next_address = ((prefetch.count > 0) ? prefetch.address[prefetch.count - 1]
                                                       : cpu.state.r15) + (thumb ? 2 : 4);
    
    prefetch.active = true;
    prefetch.address[prefetch.count] = next_address;
    prefetch.countdown = (thumb ? cycles16 : cycles32)[ARM::ACCESS_SEQ][(next_address >> 24) & 15];
    
    //std::printf("start prefetch for 0x%08x last=0x%08x\n", prefetch.address[0], last_rom_addr);
  }
  
  if (IS_ROM_REGION(address)) {
    last_rom_addr = address;
  }
  
  #undef IS_ROM_REGION
  
  /* TODO: this check does not guarantee 100% that this is an opcode fetch. */
  if (address == cpu.state.r15) {
    if (address == prefetch.address[0]) {
      cycles = 1;
      /* TODO: this is redundant and slow. */
      prefetch.count--;
      for (int i = 1; i < capacity; i++) {
        prefetch.address[i - 1] = prefetch.address[i];
      }
    } else {
      prefetch.active = false;
      prefetch.count = 0;  
    }
  }
  
  Tick(cycles);
}

void CPU::RunFor(int cycles) {
  int elapsed;

  /* Compensate for over- or undershoot from previous calls. */
  cycles += ticks_cpu_left;

  while (cycles > 0) {
    /* Run only for the duration the caller requested. */
    if (cycles < ticks_to_event) {
      ticks_to_event = cycles;
    }
    
    /* CPU may run until the next event must be executed. */
    ticks_cpu_left = ticks_to_event;
    
    /* 'ticks_cpu_left' will be consumed by memory accesses,
     * internal CPU cycles or timers during CPU idle.
     * In any case it is decremented by calls to Tick(cycles).
     */
    while (ticks_cpu_left > 0) {
      auto fire = mmio.irq_ie & mmio.irq_if;

      if (mmio.haltcnt == HaltControl::HALT && fire) {
        mmio.haltcnt = HaltControl::RUN;
      }

      /* DMA and CPU cannot run simultaneously since
       * both access the memory bus.
       * If DMA is requested the CPU will be blocked.
       */
      if (dma.IsRunning()) {
        dma.Run(ticks_cpu_left);
      } else if (mmio.haltcnt == HaltControl::RUN) {
        if (mmio.irq_ime && fire) {
          cpu.SignalIRQ();
        }
        cpu.Run();
      } else {
        /* Forward to the next event or timer IRQ. */
        Tick(std::min(timer.GetCyclesUntilIRQ(), ticks_cpu_left));
      }
    }
    
    elapsed = ticks_to_event - ticks_cpu_left;
    
    cycles -= elapsed;
    
    /* Update events and determine when the next event will happen. */
    ticks_to_event = scheduler.Schedule(elapsed);
  }
}

void CPU::UpdateCycleLUT() {
  /* TODO: implement register 0x04000800. */
  auto cycles16_n = cycles16[ARM::ACCESS_NSEQ];
  auto cycles16_s = cycles16[ARM::ACCESS_SEQ];
  auto cycles32_n = cycles32[ARM::ACCESS_NSEQ];
  auto cycles32_s = cycles32[ARM::ACCESS_SEQ];
  
  int sram_cycles = 1 + s_ws_nseq[mmio.waitcnt.sram];
  
  cycles16_n[0xE] = sram_cycles;
  cycles32_n[0xE] = sram_cycles;
  cycles16_s[0xE] = sram_cycles;
  cycles32_s[0xE] = sram_cycles;

  for (int i = 0; i < 2; i++) {
    /* ROM: WS0/WS1/WS2 non-sequential timing. */
    cycles16_n[0x8+i] = 1 + s_ws_nseq[mmio.waitcnt.ws0_n];
    cycles16_n[0xA+i] = 1 + s_ws_nseq[mmio.waitcnt.ws1_n];
    cycles16_n[0xC+i] = 1 + s_ws_nseq[mmio.waitcnt.ws2_n];
    
    /* ROM: WS0/WS1/WS2 sequential timing. */
    cycles16_s[0x8+i] = 1 + s_ws_seq0[mmio.waitcnt.ws0_s];
    cycles16_s[0xA+i] = 1 + s_ws_seq1[mmio.waitcnt.ws1_s];
    cycles16_s[0xC+i] = 1 + s_ws_seq2[mmio.waitcnt.ws2_s];
    
    /* ROM: WS0/WS1/WS2 32-bit non-sequential access: 1N access, 1S access */
    cycles32_n[0x8+i] = cycles16_n[0x8] + cycles16_s[0x8];
    cycles32_n[0xA+i] = cycles16_n[0xA] + cycles16_s[0xA];
    cycles32_n[0xC+i] = cycles16_n[0xC] + cycles16_s[0xC];
    
    /* ROM: WS0/WS1/WS2 32-bit sequential access: 2S accesses */
    cycles32_s[0x8+i] = cycles16_s[0x8] * 2;
    cycles32_s[0xA+i] = cycles16_s[0xA] * 2;
    cycles32_s[0xC+i] = cycles16_s[0xC] * 2;  
  }
}
