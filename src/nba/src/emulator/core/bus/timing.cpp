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

void Bus::PrefetchStepRAM(int cycles) {
  auto const& cpu = hw.cpu;

  if (cpu.mmio.waitcnt.prefetch) {
    auto thumb = cpu.state.cpsr.f.thumb;
    auto r15 = cpu.state.r15;

    /* During any execute cycle except for the fetch cycle, 
     * r15 will be three instruction ahead instead of two.
     */
    if (!cpu.code) {
      r15 -= thumb ? 2 : 4;
    }

    if (!prefetch.active && prefetch.rom_code_access && prefetch.count < prefetch.capacity) {
      if (prefetch.count == 0) {
        if (thumb) {
          prefetch.opcode_width = 2;
          prefetch.capacity = 8;
          prefetch.duty = wait16[int(Access::Sequential)][r15 >> 24];
        } else {
          prefetch.opcode_width = 4;
          prefetch.capacity = 4;
          prefetch.duty = wait32[int(Access::Sequential)][r15 >> 24];
        }
        prefetch.last_address = r15 + prefetch.opcode_width;
        prefetch.head_address = prefetch.last_address;
      } else {
        prefetch.last_address += prefetch.opcode_width;
      }

      prefetch.countdown = prefetch.duty;
      prefetch.active = true;
    }
  }

  Step(cycles);
}

void Bus::PrefetchStepROM(u32 address, int cycles) {
  auto const& cpu = hw.cpu;

  if (cpu.mmio.waitcnt.prefetch) {
    prefetch.rom_code_access = cpu.code;

    if (cpu.code && prefetch.count != 0) {
      if (address == prefetch.head_address) {
        prefetch.count--;
        prefetch.head_address += prefetch.opcode_width;
        PrefetchStepRAM(1);
        return;
      } else {
        prefetch.count = 0;
      }
    }

    if (prefetch.active) {
      if (cpu.code && address == prefetch.last_address) {
        Step(prefetch.countdown);
        prefetch.count--;
        return;
      }

      prefetch.active = false;
    }
  }

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

  if (prefetch.active) {
    prefetch.countdown -= cycles;

    if (prefetch.countdown <= 0) {
      prefetch.count++;
      prefetch.active = false;
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
  auto& waitcnt = hw.cpu.mmio.waitcnt;
  auto sram = nseq[waitcnt.sram];

  for (int i = 0; i < 2; i++) {
    // ROM: WS0/WS1/WS2 16-bit non-sequential access
    wait16[n][0x8 + i] = nseq[waitcnt.ws0_n];
    wait16[n][0xA + i] = nseq[waitcnt.ws1_n];
    wait16[n][0xC + i] = nseq[waitcnt.ws2_n];

    // ROM: WS0/WS1/WS2 16-bit sequential access
    wait16[s][0x8 + i] = seq0[waitcnt.ws0_s];
    wait16[s][0xA + i] = seq1[waitcnt.ws1_s];
    wait16[s][0xC + i] = seq2[waitcnt.ws2_s];

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
