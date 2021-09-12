/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "arm/arm7tdmi.hpp"
#include "bus/bus.hpp"

namespace nba::core {

void Bus::Idle() {
  Step(1);
}

void Bus::Prefetch(u32 address, int cycles) {
  if (hw.waitcnt.prefetch) {
    // TODO: distinguish code and data accesses instead of comparing against R15.
    if (address != hw.cpu.state.r15) {
      prefetch.active = false;
      prefetch.count = 0;
      Step(cycles);
      return;
    }

    // Case #1: requested address is the first entry in the prefetch buffer.
    if (prefetch.count != 0 && address == prefetch.head_address) {
      prefetch.count--;
      prefetch.head_address += prefetch.opcode_width;
      Step(1);
      return;
    }

    // Case #2: requested address is currently being prefetched.
    if (prefetch.active && address == prefetch.last_address) {
      Step(prefetch.countdown);
      prefetch.head_address = prefetch.last_address;
      prefetch.count = 0;
      return;
    }

    // Case #3: requested address is loaded through the Game Pak.
    // The prefetch unit will be engaged and keeps the burst transfer alive.
    Step(cycles);
    prefetch.active = true;
    prefetch.count = 0;
    if (hw.cpu.state.cpsr.f.thumb) {
      prefetch.opcode_width = sizeof(u16);
      prefetch.capacity = 8;
      prefetch.duty = wait16[int(Access::Sequential)][address >> 24];
    } else {
      prefetch.opcode_width = sizeof(u32);
      prefetch.capacity = 4;
      prefetch.duty = wait32[int(Access::Sequential)][address >> 24];
    }
    prefetch.countdown = prefetch.duty;
    prefetch.last_address = address + prefetch.opcode_width;
    prefetch.head_address = prefetch.last_address;
  } else {
    Step(cycles);
  }
}

void Bus::StopPrefetch() {
  if (prefetch.active) {
    // TODO: do more testing on the timing.
    Step(1);
    prefetch.active = false;
    prefetch.count = 0;
  }
}

void Bus::Step(int cycles) {
  dma.openbus = false;

  if (hw.dma.IsRunning() && !dma.active) {
    dma.active = true;
    hw.dma.Run();
    dma.active = false;
    dma.openbus = true;
  }

  scheduler.AddCycles(cycles);

  if (prefetch.active) {
    prefetch.countdown -= cycles;

    if (prefetch.countdown <= 0) {
      prefetch.count++;

      if (prefetch.count < prefetch.capacity) {
        prefetch.last_address += prefetch.opcode_width;
        prefetch.countdown += prefetch.duty;
      } else {
        prefetch.active = false;
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

  for (int i = 0; i < 2; i++) {
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
