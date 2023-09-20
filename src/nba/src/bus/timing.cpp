/*
 * Copyright (C) 2023 fleroviux
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

  // @todo: clean me up
  previous_code = false;
  previous_idle = true;
}

void Bus::StepPrefetchUnit(int cycles) {
  // Do a dum dum cycle-by-cycle simulation for now. We can fix this later.
  while(cycles-- > 0) {
    if(!prefetch.fetching) {
      return; // nothing to do!
    }

    if(--prefetch.fetch_timer == 0) { // finished fetch, latch data bus value into the buffer
      prefetch.buffer[prefetch.wr_position] = memory.rom.ReadROM16(0, true); // we do not care about the address because of sequential access

      prefetch.fetch_timer = prefetch.fetch_duty; // reload fetch timer

      if(++prefetch.wr_position == prefetch.buffer.size() || !hw.waitcnt.prefetch) {
        prefetch.fetching = false;
      }
    }
  }
}

void Bus::StopPrefetch() {
  // For now we interpret this as ROM write or SRAM read/write access

  if(prefetch.fetching) {
    prefetch.fetching = false; // must happen first

    // Concurrent access penalty
    if(prefetch.fetch_timer == 1 && hw.cpu.state.r15 >= 0x08000000 && hw.cpu.state.r15 <= 0x0DFFFFFF) { // TODO: remove R15 hack
      Step(1);
    }
  }

  // Remove me?
  prefetch.rd_position = 0U;
  prefetch.wr_position = 0U;
}

u16 Bus::ReadGamePakROM16(u32 address, int sequential, bool code) {
  // TODO: can we really detect actual CPU idle/internal cycles?

  // Try to detect CPU branches based on the previous and current access type
  if(code && (previous_code || previous_idle) && !sequential) {
    // @todo: concurrent access penalty?
    prefetch.rd_position = 0U;
    prefetch.wr_position = 0U;
    prefetch.fetching = false;
  }

  // Force non-sequential access after idle cycle
  // This must happen after the branch detection
  if(previous_idle) {
    sequential = 0;
  }

  // Read opcodes from the prefetch buffer while it holds data.
  // Assumption: this can happen as long as there are unconsumed entries in the buffer, even if the prefetch unit is not enabled anymore.
  if(code && prefetch.rd_position < prefetch.wr_position) {
    // @todo: concurrent access penalty?
    Step(1);

    const u16 half_word = prefetch.buffer[prefetch.rd_position++];

    // TEST: reset buffer once everything has been read.
    if(prefetch.rd_position == prefetch.wr_position) {
      prefetch.rd_position = 0U;
      prefetch.wr_position = 0U;
      // prefetch.fetching = false; // In case that still was true for whatever reason, don't think this really can happen.
    }

    return half_word;
  } else if(code && prefetch.fetching && prefetch.rd_position == prefetch.wr_position) {
    // Handle case where next buffer entry is currently being prefetched.

    Step(prefetch.fetch_timer);

    // TODO: deduplicate logic
    const u16 half_word = prefetch.buffer[prefetch.rd_position++];

    // TEST: reset buffer once everything has been read.
    if(prefetch.rd_position == prefetch.wr_position) {
      prefetch.rd_position = 0U;
      prefetch.wr_position = 0U;
      // prefetch.fetching = false; // In case that still was true for whatever reason, don't think this really can happen.
    }

    return half_word;
  }

  // Assumption: any data access stops prefetch
  if(!code) {
    StopPrefetch();
  }

  // For practicality we let this happen before engaging prefetch
  // Further down we assume that this access may have already went through the prefetch unit.
  Step(wait16[sequential][address >> 24]);
  const u16 half_word = memory.rom.ReadROM16(address, sequential);

  /**
   * Assumed conditions for prefetch to begin doing work:
   *  - A code ROM access is being made (can be sequential or non-sequential?)
   *  - Prefetch is enabled in WAITCNT
   *  - TODO: All entries in the prefetch buffer has been consumed after it has been stopped?
   */
  if(code && !prefetch.fetching && hw.waitcnt.prefetch) {
    // @todo: init at one or zero? we assume one because the first read likely will end up in the prefetch buffer too?
    // alternatively: should be just increment rd_position and wr_position?
    prefetch.rd_position = 1U;
    prefetch.wr_position = 1U;
    prefetch.fetching = true;
    prefetch.fetch_duty = wait16[1][8]; // WS0 sequential access timing, TODO: is this always correct? And should it be latched?!?
    prefetch.fetch_timer = prefetch.fetch_duty;
  }

  return half_word;
}

void Bus::Step(int cycles) {
  scheduler.AddCycles(cycles);

  StepPrefetchUnit(cycles);
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
