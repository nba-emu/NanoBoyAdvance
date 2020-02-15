/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
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
  : ARM7TDMI::ARM7TDMI(this)
  , config(config)
  , apu(this)
  , ppu(this)
  , dma(this)
  , timer(this)
{
  std::memset(memory.bios, 0, 0x04000);
  Reset();
}

void CPU::Reset() {
  /* TODO: properly reset the scheduler. */
  scheduler.Add(ppu.event);
  
  /* Clear all memory buffers. */
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

  mmio.rcnt_hack = 0;

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
  
  for (int i = 16; i < 256; i++) {
    cycles16[ARM::ACCESS_NSEQ][i] = 1;
    cycles16[ARM::ACCESS_SEQ ][i] = 1;
    cycles32[ARM::ACCESS_NSEQ][i] = 1;
    cycles32[ARM::ACCESS_SEQ ][i] = 1;
  }
  
  prefetch.active = false;
  prefetch.rd_pos = 0;
  prefetch.wr_pos = 0;
  prefetch.count = 0;
  last_rom_address = 0;
  
  dma.Reset();
  timer.Reset();
  apu.Reset();
  ppu.Reset();
  ARM7TDMI::Reset();
  
  if (config->skip_bios) {
    state.bank[ARM::BANK_SVC][ARM::BANK_R13] = 0x03007FE0; 
    state.bank[ARM::BANK_IRQ][ARM::BANK_R13] = 0x03007FA0;
    state.reg[13] = 0x03007F00;
    state.cpsr.f.mode = ARM::MODE_USR;
    state.r15 = 0x08000000;
  }
}

void CPU::Tick(int cycles) {
  timer.Run(cycles);
  ticks_cpu_left -= cycles;
  
  if (prefetch.active) {
    prefetch.countdown -= cycles;
    
    if (prefetch.countdown <= 0) {
      prefetch.count++;
      prefetch.wr_pos = (prefetch.wr_pos + 1) % 8;
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
  
  int thumb = state.cpsr.f.thumb;
  int capacity = thumb ? 8 : 4;
  
  if (prefetch.active) {
    /* If prefetching the desired opcode just complete. */
    if (address == prefetch.address[prefetch.wr_pos]) {
      int count  = prefetch.count;
      int wr_pos = prefetch.wr_pos;
      
      Tick(prefetch.countdown);
      
      // HACK: overwrite count and wr_pos with old values to
      // account for the prefetched opcode being consumed right away.
      prefetch.count = count;
      prefetch.wr_pos = wr_pos;
      
      last_rom_address = address;
      return;
    }
    
    if (IS_ROM_REGION(address)) {
      prefetch.active = false;
    }
  } else if (prefetch.count < capacity &&
             IS_ROM_REGION(state.r15) &&
            !IS_ROM_REGION(address) && 
             state.r15 == last_rom_address) {
    std::uint32_t next_address;
    
    if (prefetch.count > 0) {
      next_address = prefetch.last_address;
    } else {
      next_address = state.r15;
    }
    
    next_address += thumb ? 2 : 4;
    prefetch.last_address = next_address;
    
    prefetch.active = true;
    prefetch.address[prefetch.wr_pos] = next_address;
    prefetch.countdown = (thumb ? cycles16 : cycles32)[ARM::ACCESS_SEQ][next_address >> 24];
  }
  
  if (IS_ROM_REGION(address)) {
    last_rom_address = address;
  }
  
  #undef IS_ROM_REGION
  
  /* TODO: this check does not guarantee 100% that this is an opcode fetch. */
  if (prefetch.count > 0 && address == state.r15) {
    if (address == prefetch.address[prefetch.rd_pos]) {
      cycles = 1;
      prefetch.count--;
      prefetch.rd_pos = (prefetch.rd_pos + 1) % 8;
    } else {
      prefetch.active = false;
      prefetch.count = 0;
      prefetch.rd_pos = 0;
      prefetch.wr_pos = 0;
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
          SignalIRQ();
        }
        Run();
      } else {
        /* Forward to the next event or timer IRQ. */
        Tick(std::min(timer.EstimateCyclesUntilIRQ(), ticks_cpu_left));
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
