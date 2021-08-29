/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "cpu.hpp"

#include <common/compiler.hpp>
#include <common/crc32.hpp>
#include <cstring>

namespace nba::core {

using Key = InputDevice::Key;

CPU::CPU(std::shared_ptr<Config> config)
    : ARM7TDMI::ARM7TDMI(scheduler, bus)
    , config(config)
    , irq(*this, scheduler)
    , dma(bus, irq, scheduler)
    , apu(scheduler, dma, *this, config)
    , ppu(scheduler, irq, dma, config)
    , timer(scheduler, irq, apu)
    , keypad(irq, config)
    , bus(scheduler, {*this, irq, dma, apu, ppu, timer, keypad}) {
  Reset();
}

void CPU::Reset() {
  scheduler.Reset();
  irq.Reset();
  dma.Reset();
  timer.Reset();
  apu.Reset();
  ppu.Reset();
  ARM7TDMI::Reset();
  bus.Reset();
  keypad.Reset();

  if (config->skip_bios) {
    SwitchMode(arm::MODE_SYS);
    state.bank[arm::BANK_SVC][arm::BANK_R13] = 0x03007FE0;
    state.bank[arm::BANK_IRQ][arm::BANK_R13] = 0x03007FA0;
    state.r13 = 0x03007F00;
    state.r15 = 0x08000000;
  }

  mp2k_soundmain_address = 0xFFFFFFFF;
  if (config->audio.mp2k_hle_enable) {
    MP2KSearchSoundMainRAM();
    apu.GetMP2K().UseCubicFilter() = config->audio.mp2k_hle_cubic;
  }
}

void CPU::RunFor(int cycles) {
  using HaltControl = Bus::Hardware::HaltControl;

  auto limit = scheduler.GetTimestampNow() + cycles;

  while (scheduler.GetTimestampNow() < limit) {
    if (unlikely(bus.hw.haltcnt == HaltControl::Halt && irq.HasServableIRQ())) {
      bus.hw.haltcnt = HaltControl::Run;
    }

    if (likely(bus.hw.haltcnt == HaltControl::Run)) {
      if (state.r15 == mp2k_soundmain_address) {
        MP2KOnSoundMainRAMCalled();  
      }
      Run();
    } else {
      bus.Step(scheduler.GetRemainingCycleCount());
    }
  }
}

void CPU::MP2KSearchSoundMainRAM() {
  static constexpr u32 kSoundMainCRC32 = 0x27EA7FCF;
  static constexpr int kSoundMainLength = 48;

  auto& game_pak = bus.memory.game_pak;
  auto& rom = game_pak.GetRawROM();
  u32 address_max = rom.size() - kSoundMainLength;

  for (u32 address = 0; address <= address_max; address += 2) {
    auto crc = crc32(&rom[address], kSoundMainLength);

    if (crc == kSoundMainCRC32) {
      /* We have found SoundMain().
       * The pointer to SoundMainRAM() is stored at offset 0x74.
       */
      mp2k_soundmain_address = read<u32>(rom.data(), address + 0x74);

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
  MP2K::SoundInfo* sound_info;
  auto address = Read<u32, true>(0x03007FF0);

  // Get host pointer to SoundMain structure.
  switch (address >> 24) {
    case 0x02: {
      sound_info = (MP2K::SoundInfo*)&bus.memory.wram[address & 0x3FFFF];
      break;
    }
    case 0x03: {
      sound_info = (MP2K::SoundInfo*)&bus.memory.iram[address & 0x7FFF];
      break;
    }
    default: {
      ASSERT(false, "MP2K HLE: SoundInfo structure is at unsupported address 0x{0:08X}.", address);
      break;
    }
  }

  apu.GetMP2K().SoundMainRAM(*sound_info);
}

} // namespace nba::core