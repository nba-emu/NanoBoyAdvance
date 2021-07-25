/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "cpu.hpp"

#include <common/compiler.hpp>
#include <cstring>

namespace nba::core {

constexpr int CPU::s_ws_nseq[4];
constexpr int CPU::s_ws_seq0[2];
constexpr int CPU::s_ws_seq1[2];
constexpr int CPU::s_ws_seq2[2];

using Key = InputDevice::Key;

CPU::CPU(std::shared_ptr<Config> config)
    : ARM7TDMI::ARM7TDMI(scheduler, this)
    , config(config)
    , irq(*this, scheduler)
    , dma(*this, irq, scheduler)
    , apu(scheduler, dma, *this, config)
    , ppu(scheduler, irq, dma, config)
    , timer(scheduler, irq, apu)
    , serial_bus(irq) {
  std::memset(memory.bios, 0, 0x04000);
  Reset();
}

void CPU::Reset() {
  std::memset(memory.wram, 0, 0x40000);
  std::memset(memory.iram, 0, 0x08000);

  mmio = {};
  prefetch = {};
  bus_is_controlled_by_dma = false;
  openbus_from_dma = false;
  UpdateMemoryDelayTable();

  for (int i = 16; i < 256; i++) {
    cycles16[int(Access::Nonsequential)][i] = 1;
    cycles32[int(Access::Nonsequential)][i] = 1;
    cycles16[int(Access::Sequential)][i] = 1;
    cycles32[int(Access::Sequential)][i] = 1;
  }

  scheduler.Reset();
  irq.Reset();
  dma.Reset();
  timer.Reset();
  apu.Reset();
  ppu.Reset();
  serial_bus.Reset();
  ARM7TDMI::Reset();

  if (config->skip_bios) {
    SwitchMode(arm::MODE_SYS);
    state.bank[arm::BANK_SVC][arm::BANK_R13] = 0x03007FE0;
    state.bank[arm::BANK_IRQ][arm::BANK_R13] = 0x03007FA0;
    state.r13 = 0x03007F00;
    state.r15 = 0x08000000;
  }

  config->input_dev->SetOnChangeCallback(std::bind(&CPU::OnKeyPress,this));

  mp2k_soundmain_address = 0xFFFFFFFF;
  if (config->audio.m4a_xq_enable) {
    MP2KSearchSoundMainRAM();
  }
}

void CPU::RunFor(int cycles) {
  auto limit = scheduler.GetTimestampNow() + cycles;

  while (scheduler.GetTimestampNow() < limit) {
    if (unlikely(mmio.haltcnt == HaltControl::HALT && irq.HasServableIRQ())) {
      mmio.haltcnt = HaltControl::RUN;
    }

    if (likely(mmio.haltcnt == HaltControl::RUN)) {
      if (state.r15 == mp2k_soundmain_address) {
        MP2KOnSoundMainRAMCalled();  
      }
      Run();
    } else {
      Tick(scheduler.GetRemainingCycleCount());
    }
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

void CPU::OnKeyPress() {
  mmio.keyinput = 0;

  if (!config->input_dev->Poll(Key::A)) mmio.keyinput |= 1;
  if (!config->input_dev->Poll(Key::B)) mmio.keyinput |= 2;
  if (!config->input_dev->Poll(Key::Select)) mmio.keyinput |= 4;
  if (!config->input_dev->Poll(Key::Start)) mmio.keyinput |= 8;
  if (!config->input_dev->Poll(Key::Right)) mmio.keyinput |= 16;
  if (!config->input_dev->Poll(Key::Left)) mmio.keyinput |= 32;
  if (!config->input_dev->Poll(Key::Up)) mmio.keyinput |= 64;
  if (!config->input_dev->Poll(Key::Down)) mmio.keyinput |= 128;
  if (!config->input_dev->Poll(Key::R)) mmio.keyinput |= 256;
  if (!config->input_dev->Poll(Key::L)) mmio.keyinput |= 512;

  CheckKeypadInterrupt();
}

void CPU::CheckKeypadInterrupt() {
  const auto& keycnt = mmio.keycnt;
  const auto keyinput = ~mmio.keyinput & 0x3FF;

  if (!keycnt.interrupt) {
    return;
  }
  
  if (keycnt.and_mode) {
    if (keycnt.input_mask == keyinput) {
      irq.Raise(IRQ::Source::Keypad);
    }
  } else if ((keycnt.input_mask & keyinput) != 0) {
    irq.Raise(IRQ::Source::Keypad);
  }
}

void CPU::MP2KSearchSoundMainRAM() {
  // TODO: how much of the function should we include?
  static const u8 pattern[] = {
    0x1A, 0x48, 0x00, 0x68, 0x1A, 0x4A, 0x03, 0x68,
    0x9A, 0x42, 0x00, 0xD0, 0x70, 0x47, 0x01, 0x33,
    0x03, 0x60, 0xF0, 0xB5, 0x41, 0x46, 0x4A, 0x46,
    0x53, 0x46, 0x5C, 0x46, 0x1F, 0xB4, 0x86, 0xB0,
    0x01, 0x7B, 0x00, 0x29
  };

  auto& rom = game_pak.GetRawROM();

  for (u32 address = 0; address < rom.size(); address += 2) {
    bool match = true;

    for (int i = 0; i < sizeof(pattern); i++) {
      if (rom[address + i] != pattern[i]) {
        match = false;
        break;
      }
    }

    if (match) {
      // We have found SoundMain(), the address of SoundMainRAM()
      // is stored relative to SoundMain().
      mp2k_soundmain_address = common::read<u32>(rom.data(), address + 0x74);

      LOG_INFO("MP2K: found SoundMainRAM() routine at 0x{0:08X}.", mp2k_soundmain_address);

      if (mp2k_soundmain_address & 1) {
        mp2k_soundmain_address &= ~1;
        mp2k_soundmain_address += sizeof(u16) * 2;
      } else {
        mp2k_soundmain_address &= ~3;
        mp2k_soundmain_address += sizeof(u32) * 2;
      }
      return;
    }
  }
}

void CPU::MP2KOnSoundMainRAMCalled() {
  M4ASoundInfo* sound_info;
  auto address = ReadWord(0x03007FF0, Access::Sequential);

  // Get host pointer to SoundMain structure.
  switch (address >> 24) {
    case 0x02: {
      sound_info = (M4ASoundInfo*)&memory.wram[address & 0x3FFFF];
      break;
    }
    case 0x03: {
      sound_info = (M4ASoundInfo*)&memory.iram[address & 0x7FFF];
      break;
    }
    default: {
      ASSERT(false, "MP2K HLE: SoundInfo structure is at unsupported address 0x{0:08X}.", address);
      break;
    }
  }

  apu.MP2KSoundMainRAM(*sound_info);
}

} // namespace nba::core
