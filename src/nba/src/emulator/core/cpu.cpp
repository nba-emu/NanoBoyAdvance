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
    : ARM7TDMI::ARM7TDMI(scheduler, &bus)
    , config(config)
    , irq(*this, scheduler)
    , dma(bus, irq, scheduler)
    , apu(scheduler, dma, *this, config)
    , ppu(scheduler, irq, dma, config)
    , timer(scheduler, irq, apu)
    , serial_bus(irq)
    , bus(scheduler, {*this, irq, dma, apu, ppu, timer, serial_bus}) {
  std::memset(memory.bios, 0, 0x04000);
  Reset();
}

void CPU::Reset() {
  std::memset(memory.wram, 0, 0x40000);
  std::memset(memory.iram, 0, 0x08000);

  mmio = {};

  scheduler.Reset();
  irq.Reset();
  dma.Reset();
  timer.Reset();
  apu.Reset();
  ppu.Reset();
  serial_bus.Reset();
  ARM7TDMI::Reset();
  bus.Reset();

  if (config->skip_bios) {
    SwitchMode(arm::MODE_SYS);
    state.bank[arm::BANK_SVC][arm::BANK_R13] = 0x03007FE0;
    state.bank[arm::BANK_IRQ][arm::BANK_R13] = 0x03007FA0;
    state.r13 = 0x03007F00;
    state.r15 = 0x08000000;
  }

  config->input_dev->SetOnChangeCallback(std::bind(&CPU::OnKeyPress,this));

  mp2k_soundmain_address = 0xFFFFFFFF;
  if (config->audio.mp2k_hle_enable) {
    MP2KSearchSoundMainRAM();
    apu.GetMP2K().UseCubicFilter() = config->audio.mp2k_hle_cubic;
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
      bus.PrefetchStepRAM(scheduler.GetRemainingCycleCount());
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

void CPU::MP2KSearchSoundMainRAM() {
  static constexpr u32 kSoundMainCRC32 = 0x27EA7FCF;
  static constexpr int kSoundMainLength = 48;

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
      sound_info = (MP2K::SoundInfo*)&memory.wram[address & 0x3FFFF];
      break;
    }
    case 0x03: {
      sound_info = (MP2K::SoundInfo*)&memory.iram[address & 0x7FFF];
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
