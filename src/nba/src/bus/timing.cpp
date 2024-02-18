/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "arm/arm7tdmi.hpp"
#include "bus/bus.hpp"

namespace nba::core {

void Bus::Idle() {
  /**
   * The CPU can run in parallel to DMA while it executes internal cycles.
   * It will only be stalled (for the remaining duration of DMA) once it accesses the bus again.
   *
   * In this emulator we unfortunately cannot cycle CPU and DMA in parallel.
   * Instead, we track DMA duration when DMA becomes active on an internal CPU cycle.
   * We then treat subsequent internal CPU cycles as free until the CPU accesses the bus or
   * the total number of internal CPU cycles would exceed the DMA duration.
   *
   * Fortunately this should have no observable side-effects! 
   */

  if(hw.dma.IsRunning()) {
    parallel_internal_cpu_cycle_limit = hw.dma.Run();
  }

  if(parallel_internal_cpu_cycle_limit == 0) {
    Step(1);
  } else {
    parallel_internal_cpu_cycle_limit--;
  }
}

void Bus::Prefetch(u32 address, bool code, int cycles) {
  if(!code) {
    StopPrefetch();
    Step(cycles);
    return;
  }

  if(prefetch.active) {
    // Case #1: requested address is the first entry in the prefetch buffer.
    if(prefetch.count != 0 && address == prefetch.head_address) {
      prefetch.count--;
      prefetch.head_address += prefetch.opcode_width;
      Step(1);
      return;
    }

    // Case #2: requested address is currently being prefetched.
    if(prefetch.countdown > 0 && address == prefetch.last_address) {
      Step(prefetch.countdown);
      prefetch.head_address = prefetch.last_address;
      prefetch.count = 0;
      return;
    }
  }

  const int page = address >> 24;

  StopPrefetch();

  // Case #3: requested address is loaded through the Game Pak.
  if(hw.prefetch_buffer_was_disabled) {
    // force the access to be non-sequential.
    // @todo: make this less dodgy.
         if(cycles == wait16[1][page]) cycles = wait16[0][8];
    else if(cycles == wait32[1][page]) cycles = wait32[0][8];

    hw.prefetch_buffer_was_disabled = false;
  }
  Step(cycles);
  if(hw.waitcnt.prefetch) {
    const bool thumb = hw.cpu.state.cpsr.f.thumb;

    // The prefetch unit will be engaged and keeps the burst transfer alive.
    prefetch.active = true;
    prefetch.count = 0;
    prefetch.thumb = thumb;
    if(thumb) {
      prefetch.opcode_width = sizeof(u16);
      prefetch.capacity = 8;
      prefetch.duty = wait16[int(Access::Sequential)][page];
    } else {
      prefetch.opcode_width = sizeof(u32);
      prefetch.capacity = 4;
      prefetch.duty = wait32[int(Access::Sequential)][page];
    }
    prefetch.countdown = prefetch.duty;
    prefetch.last_address = address + prefetch.opcode_width;
    prefetch.head_address = prefetch.last_address;
  }
}

void Bus::StopPrefetch() {
  if(prefetch.active) {
    u32 r15 = hw.cpu.state.r15;

    /* If ROM data/SRAM/FLASH is accessed in a cycle, where the prefetch unit
     * is active and finishing a half-word access, then a one-cycle penalty applies.
     * Note: the prefetch unit is only active when executing code from ROM.
     */
    if(r15 >= 0x08000000 && r15 <= 0x0DFFFFFF) {
      auto half_duty_plus_one = (prefetch.duty >> 1) + 1;
      auto countdown = prefetch.countdown;

      if(countdown == 1 || (!prefetch.thumb && countdown == half_duty_plus_one)) {
        Step(1);
      }
    }

    prefetch.active = false;
  }
}

void Bus::Step(int cycles) {
  scheduler.AddCycles(cycles);

  if(prefetch.active) {
    prefetch.countdown -= cycles;

    while(prefetch.countdown <= 0) {
      prefetch.count++;

      if(hw.waitcnt.prefetch && prefetch.count < prefetch.capacity) {
        prefetch.last_address += prefetch.opcode_width;
        prefetch.countdown += prefetch.duty;
      } else {
        break;
      }
    }
  }
}

void Bus::UpdateWaitStateTable() {
  static constexpr int nseq[4] = { 5, 4, 3, 9 };
  static constexpr int seq0[2] = { 3, 2 };
  static constexpr int seq1[2] = { 5, 2 };
  static constexpr int seq2[2] = { 9, 2 };

  auto n = int(Access::Nonsequential);
  auto s = int(Access::Sequential);
  auto& waitcnt = hw.waitcnt;
  auto sram = nseq[waitcnt.sram];

  for(int i = 0; i < 2; i++) {
    // ROM: WS0/WS1/WS2 16-bit non-sequential access
    wait16[n][0x8 + i] = nseq[waitcnt.ws0[n]];
    wait16[n][0xA + i] = nseq[waitcnt.ws1[n]];
    wait16[n][0xC + i] = nseq[waitcnt.ws2[n]];

    // ROM: WS0/WS1/WS2 16-bit sequential access
    wait16[s][0x8 + i] = seq0[waitcnt.ws0[s]];
    wait16[s][0xA + i] = seq1[waitcnt.ws1[s]];
    wait16[s][0xC + i] = seq2[waitcnt.ws2[s]];

    // ROM: WS0/WS1/WS2 32-bit non-sequential access: 1N access, 1S access
    wait32[n][0x8 + i] = wait16[n][0x8] + wait16[s][0x8];
    wait32[n][0xA + i] = wait16[n][0xA] + wait16[s][0xA];
    wait32[n][0xC + i] = wait16[n][0xC] + wait16[s][0xC];

    // ROM: WS0/WS1/WS2 32-bit sequential access: 2S accesses
    wait32[s][0x8 + i] = wait16[s][0x8] * 2;
    wait32[s][0xA + i] = wait16[s][0xA] * 2;
    wait32[s][0xC + i] = wait16[s][0xC] * 2;

    // SRAM
    wait16[n][0xE + i] = sram;
    wait32[n][0xE + i] = sram;
    wait16[s][0xE + i] = sram;
    wait32[s][0xE + i] = sram;
  }
}

} // namespace nba::core
