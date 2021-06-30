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
    , apu(scheduler, dma, config)
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

  m4a_soundinfo = nullptr;
  m4a_original_freq = 0;
  if (config->audio.m4a_xq_enable) {
    M4ASearchForSampleFreqSet();
  }

  config->input_dev->SetOnChangeCallback(std::bind(&CPU::OnKeyPress,this));
}

void CPU::RunFor(int cycles) {
  bool m4a_xq_enable = config->audio.m4a_xq_enable && m4a_setfreq_address != 0;
  if (m4a_xq_enable && m4a_soundinfo != nullptr) {
    M4AFixupPercussiveChannels();
  }

  auto limit = scheduler.GetTimestampNow() + cycles;

  while (scheduler.GetTimestampNow() < limit) {
    if (unlikely(mmio.haltcnt == HaltControl::HALT && irq.HasServableIRQ())) {
      mmio.haltcnt = HaltControl::RUN;
    }

    if (likely(mmio.haltcnt == HaltControl::RUN)) {
      if (unlikely(m4a_xq_enable && state.r15 == m4a_setfreq_address)) {
        M4ASampleFreqSetHook();
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

void CPU::M4ASearchForSampleFreqSet() {
  static const u8 pattern[] = {
    0x53, 0x6D, 0x73, 0x68, 0x70, 0xB5, 0x02, 0x1C,
    0x1E, 0x48, 0x04, 0x68, 0xF0, 0x20, 0x00, 0x03,
    0x10, 0x40, 0x02, 0x0C
  };

  auto& rom = game_pak.GetRawROM();

  for (u32 i = 0; i < rom.size(); i++) {
    bool match = true;
    for (int j = 0; j < sizeof(pattern); j++) {
      if (rom[i + j] != pattern[j]) {
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

  u32 soundinfo_p1 = common::read<u32>(game_pak.GetRawROM().data(), (m4a_setfreq_address & 0x00FFFFFF) + 492);
  u32 soundinfo_p2;
  LOG_INFO("M4A SoundInfo pointer at 0x{0:08X}", soundinfo_p1);

  switch (soundinfo_p1 >> 24) {
    case 0x02:
      soundinfo_p2 = common::read<u32>(memory.wram, soundinfo_p1 & 0x00FFFFFF);
      break;
    case 0x03:
      soundinfo_p2 = common::read<u32>(memory.iram, soundinfo_p1 & 0x00FFFFFF);
      break;
    default:
      LOG_ERROR("M4A SoundInfo pointer is outside of IWRAM or EWRAM, unsupported.");
      return;
  }

  LOG_INFO("M4A SoundInfo address is 0x{0:08X}", soundinfo_p2);

  switch (soundinfo_p2 >> 24) {
    case 0x02:
      m4a_soundinfo = reinterpret_cast<M4ASoundInfo*>(memory.wram + (soundinfo_p2 & 0x00FFFFFF));
      break;
    case 0x03:
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

} // namespace nba::core
