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

namespace nba::core {

constexpr int CPU::s_ws_nseq[4];
constexpr int CPU::s_ws_seq0[2];
constexpr int CPU::s_ws_seq1[2];
constexpr int CPU::s_ws_seq2[2];

CPU::CPU(std::shared_ptr<Config> config)
  : ARM7TDMI::ARM7TDMI(this)
  , config(config)
  , dma(this, &irq_controller, &scheduler_new)
  , apu(&scheduler_new, &dma, config)
  , ppu(&scheduler_new, &irq_controller, &dma, config)
  , timer(&irq_controller, &apu)
{
  std::memset(memory.bios, 0, 0x04000);
  memory.rom.size = 0;
  memory.rom.mask = 0;
  Reset();
}

void CPU::Reset() {
  std::memset(memory.wram, 0, 0x40000);
  std::memset(memory.iram, 0, 0x08000);

  irq = {};
  mmio = {};
  prefetch = {};
  last_rom_address = 0;
  UpdateMemoryDelayTable();

  for (int i = 16; i < 256; i++) {
    cycles16[int(Access::Nonsequential)][i] = 1;
    cycles32[int(Access::Nonsequential)][i] = 1;
    cycles16[int(Access::Sequential)][i] = 1;
    cycles32[int(Access::Sequential)][i] = 1;
  }

  scheduler_new.Reset();
  irq_controller.Reset();
  dma.Reset();
  timer.Reset();
  apu.Reset();
  ppu.Reset();
  ARM7TDMI::Reset();

  if (config->skip_bios) {
    state.bank[arm::BANK_SVC][arm::BANK_R13] = 0x03007FE0;
    state.bank[arm::BANK_IRQ][arm::BANK_R13] = 0x03007FA0;
    state.reg[13] = 0x03007F00;
    state.cpsr.f.mode = arm::MODE_SYS;
    state.r15 = 0x08000000;
  }

  m4a_soundinfo = nullptr;
  m4a_original_freq = 0;
  if (config->audio.m4a_xq_enable && memory.rom.data != nullptr) {
    M4ASearchForSampleFreqSet();
  }
}

void CPU::Tick(int cycles) {
  if (irq.processing && irq.countdown >= 0) {
    irq.countdown -= cycles;
  }

  timer.Run(cycles);
  scheduler_new.AddCycles(cycles);

  if (prefetch.active) {
    prefetch.countdown -= cycles;

    if (prefetch.countdown <= 0) {
      prefetch.count++;
      prefetch.active = false;
    }
  }
}

void CPU::Idle() {
  PrefetchStepRAM(1);
}

void CPU::PrefetchStepRAM(int cycles) {
  if (!mmio.waitcnt.prefetch) {
    Tick(cycles);
    return;
  }

  int thumb = state.cpsr.f.thumb;

  if (!prefetch.active && prefetch.count < prefetch.capacity && state.r15 == last_rom_address) {
    if (prefetch.count == 0) {
      if (thumb) {
        prefetch.opcode_width = 2;
        prefetch.capacity = 8;
        prefetch.duty = cycles16[int(Access::Sequential)][state.r15 >> 24];
      } else {
        prefetch.opcode_width = 4;
        prefetch.capacity = 4;
        prefetch.duty = cycles32[int(Access::Sequential)][state.r15 >> 24];
      }
      prefetch.last_address = state.r15 + prefetch.opcode_width;
      prefetch.head_address = prefetch.last_address;
    } else {
      prefetch.last_address += prefetch.opcode_width;
    }

    prefetch.countdown = prefetch.duty;
    prefetch.active = true;
  }

  Tick(cycles);
}

void CPU::PrefetchStepROM(std::uint32_t address, int cycles) {
  if (!mmio.waitcnt.prefetch) {
    Tick(cycles);
    return;
  }

  if (prefetch.active) {
    /* If prefetching the desired opcode just complete. */
    if (address == prefetch.last_address) {
      int count = prefetch.count;

      Tick(prefetch.countdown);

      // HACK: overwrite count with its old value to
      // account for the prefetched opcode being consumed right away.
      prefetch.count = count;

      last_rom_address = address;
      return;
    }

    prefetch.active = false;
  }

  last_rom_address = address;

  /* TODO: this check does not guarantee 100% that this is an opcode fetch. */
  if (prefetch.count > 0 && address == state.r15) {
    if (address == prefetch.head_address) {
      prefetch.count--;
      prefetch.head_address += prefetch.opcode_width;
      PrefetchStepRAM(1);
      return;
    } else {
      prefetch.active = false;
      prefetch.count = 0;
    }
  }

  Tick(cycles);
}

void CPU::RunFor(int cycles) {
  bool m4a_xq_enable = config->audio.m4a_xq_enable && m4a_setfreq_address != 0;

  // TODO: this could end up very slow if RunFor is called too often per second.
  if (m4a_xq_enable && m4a_soundinfo != nullptr) {
    M4AFixupPercussiveChannels();
  }

  auto limit = scheduler_new.GetTimestampNow() + cycles;

  // TODO: account for per frame overshoot.
  while (scheduler_new.GetTimestampNow() < limit) {
    // TODO: optimize the std::min by updating the result whenever it changes.
    while (scheduler_new.GetTimestampNow() < std::min(scheduler_new.GetTimestampTarget(), limit)) {
      // TODO: eventually move the loop body into a separate method?
      auto has_servable_irq = irq_controller.HasServableIRQ();

      if (mmio.haltcnt == HaltControl::HALT && has_servable_irq) {
        mmio.haltcnt = HaltControl::RUN;
      }

      /* DMA and CPU cannot run simultaneously since
       * both access the memory bus.
       * If DMA is requested the CPU will be blocked.
       */
      if (dma.IsRunning()) {
        dma.Run();
      } else if (mmio.haltcnt == HaltControl::RUN) {
        if (irq_controller.MasterEnable() && has_servable_irq) {
          if (!irq.processing) {
            irq.processing = true;
            irq.countdown = 3;
          } else if (irq.countdown < 0) {
            SignalIRQ();
          }
        } else {
          irq.processing = false;
        }
        if (m4a_xq_enable && state.r15 == m4a_setfreq_address) {
          M4ASampleFreqSetHook();
        }
        Run();
      } else {
        /* Forward to the next event or timer IRQ. */
        Tick(std::min(timer.EstimateCyclesUntilIRQ(), scheduler_new.GetRemainingCycleCount()));
      }
    }

    scheduler_new.Step();
  }
}

void CPU::UpdateMemoryDelayTable() {
  auto cycles16_n = cycles16[int(Access::Nonsequential)];
  auto cycles16_s = cycles16[int(Access::Sequential)];
  auto cycles32_n = cycles32[int(Access::Nonsequential)];
  auto cycles32_s = cycles32[int(Access::Sequential)];

  int sram_cycles = 1 + s_ws_nseq[mmio.waitcnt.sram];

  cycles16_n[0xE] = sram_cycles;
  cycles32_n[0xE] = sram_cycles;
  cycles16_s[0xE] = sram_cycles;
  cycles32_s[0xE] = sram_cycles;

  for (int i = 0; i < 2; i++) {
    /* ROM: WS0/WS1/WS2 non-sequential timing. */
    cycles16_n[0x8 + i] = 1 + s_ws_nseq[mmio.waitcnt.ws0_n];
    cycles16_n[0xA + i] = 1 + s_ws_nseq[mmio.waitcnt.ws1_n];
    cycles16_n[0xC + i] = 1 + s_ws_nseq[mmio.waitcnt.ws2_n];

    /* ROM: WS0/WS1/WS2 sequential timing. */
    cycles16_s[0x8 + i] = 1 + s_ws_seq0[mmio.waitcnt.ws0_s];
    cycles16_s[0xA + i] = 1 + s_ws_seq1[mmio.waitcnt.ws1_s];
    cycles16_s[0xC + i] = 1 + s_ws_seq2[mmio.waitcnt.ws2_s];

    /* ROM: WS0/WS1/WS2 32-bit non-sequential access: 1N access, 1S access */
    cycles32_n[0x8 + i] = cycles16_n[0x8] + cycles16_s[0x8];
    cycles32_n[0xA + i] = cycles16_n[0xA] + cycles16_s[0xA];
    cycles32_n[0xC + i] = cycles16_n[0xC] + cycles16_s[0xC];

    /* ROM: WS0/WS1/WS2 32-bit sequential access: 2S accesses */
    cycles32_s[0x8 + i] = cycles16_s[0x8] * 2;
    cycles32_s[0xA + i] = cycles16_s[0xA] * 2;
    cycles32_s[0xC + i] = cycles16_s[0xC] * 2;
  }
}

void CPU::M4ASearchForSampleFreqSet() {
  static const std::uint8_t pattern[] = {
    0x53, 0x6D, 0x73, 0x68, 0x70, 0xB5, 0x02, 0x1C,
    0x1E, 0x48, 0x04, 0x68, 0xF0, 0x20, 0x00, 0x03,
    0x10, 0x40, 0x02, 0x0C
  };
  for (std::uint32_t i = 0; i < memory.rom.size; i++) {
    bool match = true;
    for (int j = 0; j < sizeof(pattern); j++) {
      if (memory.rom.data[i + j] != pattern[j]) {
        match = false;
        i += j;
        break;
      }
    }
    if (match) {
      m4a_setfreq_address = i + 0x08000008;
      LOG_INFO("Found M4A SetSampleFreq() routine at 0x{0:08X}.", m4a_setfreq_address);
      return;
    }
  }
}

void CPU::M4ASampleFreqSetHook() {
  static const int frequency_tab[16] = {
    0, 5734, 7884, 10512,
    13379, 15768, 18157, 21024,
    26758, 31536, 36314, 40137,
    42048, 0, 0, 0
  };

  LOG_INFO("M4A SampleFreqSet() called: r0 = 0x{0:08X}", state.r0);

  m4a_original_freq = frequency_tab[(state.r0 >> 16) & 15];
  state.r0 = 0x00090000;
  m4a_soundinfo = nullptr;

  std::uint32_t soundinfo_p1 = Read<std::uint32_t>(memory.rom.data.get(), (m4a_setfreq_address & 0x00FFFFFF) + 492);
  std::uint32_t soundinfo_p2;
  LOG_INFO("M4A SoundInfo pointer at 0x{0:08X}", soundinfo_p1);

  switch (soundinfo_p1 >> 24) {
    case REGION_EWRAM:
      soundinfo_p2 = Read<std::uint32_t>(memory.wram, soundinfo_p1 & 0x00FFFFFF);
      break;
    case REGION_IWRAM:
      soundinfo_p2 = Read<std::uint32_t>(memory.iram, soundinfo_p1 & 0x00FFFFFF);
      break;
    default:
      LOG_ERROR("M4A SoundInfo pointer is outside of IWRAM or EWRAM, unsupported.");
      return;
  }

  LOG_INFO("M4A SoundInfo address is 0x{0:08X}", soundinfo_p2);

  switch (soundinfo_p2 >> 24) {
    case REGION_EWRAM:
      m4a_soundinfo = reinterpret_cast<M4ASoundInfo*>(memory.wram + (soundinfo_p2 & 0x00FFFFFF));
      break;
    case REGION_IWRAM:
      m4a_soundinfo = reinterpret_cast<M4ASoundInfo*>(memory.iram + (soundinfo_p2 & 0x00FFFFFF));
      break;
    default:
      LOG_ERROR("M4A SoundInfo is outside of IWRAM or EWRAM, unsupported.");
      return;
  }
}

void CPU::M4AFixupPercussiveChannels() {
  for (int i = 0; i < kM4AMaxDirectSoundChannels; i++) {
    if (m4a_soundinfo->channels[i].type == 8) {
      m4a_soundinfo->channels[i].type = 0;
      m4a_soundinfo->channels[i].freq = m4a_original_freq;
    }
  }
}

} // namespace nba::core
