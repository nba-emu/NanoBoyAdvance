/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "bus.hpp"
#include "emulator/core/cpu.hpp"

namespace nba::core {

void Bus::Idle() {
  PrefetchStepRAM(1);
}

void Bus::PrefetchStepROM(u32 address, int cycles) {
  PrefetchStepRAM(cycles);
}

void Bus::PrefetchStepRAM(int cycles) {
  Step(cycles);
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
}

void Bus::UpdateWaitStateTable() {
  static constexpr int nseq[4] = { 5, 4, 3, 9 };
  static constexpr int seq0[2] = { 3, 2 };
  static constexpr int seq1[2] = { 5, 2 };
  static constexpr int seq2[2] = { 9, 2 };

  auto n = int(Access::Nonsequential);
  auto s = int(Access::Sequential);
  auto& waitcnt = hw.cpu.mmio.waitcnt;
  auto sram = nseq[waitcnt.sram];

  for (int i = 0; i < 2; i++) {
    // ROM: WS0/WS1/WS2 16-bit non-sequential access
    cycles16[n][0x8 + i] = nseq[waitcnt.ws0_n];
    cycles16[n][0xA + i] = nseq[waitcnt.ws1_n];
    cycles16[n][0xC + i] = nseq[waitcnt.ws2_n];

    // ROM: WS0/WS1/WS2 16-bit sequential access
    cycles16[s][0x8 + i] = seq0[waitcnt.ws0_s];
    cycles16[s][0xA + i] = seq1[waitcnt.ws1_s];
    cycles16[s][0xC + i] = seq2[waitcnt.ws2_s];

    // ROM: WS0/WS1/WS2 32-bit non-sequential access: 1N access, 1S access
    cycles32[n][0x8 + i] = cycles16[n][0x8] + cycles16[s][0x8];
    cycles32[n][0xA + i] = cycles16[n][0xA] + cycles16[s][0xA];
    cycles32[n][0xC + i] = cycles16[n][0xC] + cycles16[s][0xC];

    // ROM: WS0/WS1/WS2 32-bit sequential access: 2S accesses
    cycles32[s][0x8 + i] = cycles16[s][0x8] * 2;
    cycles32[s][0xA + i] = cycles16[s][0xA] * 2;
    cycles32[s][0xC + i] = cycles16[s][0xC] * 2;

    // SRAM
    cycles16[n][0xE + i] = sram;
    cycles32[n][0xE + i] = sram;
    cycles16[s][0xE + i] = sram;
    cycles32[s][0xE + i] = sram;
  }
}

} // namespace nba::core
