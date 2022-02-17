/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/common/crc32.hpp>

#include "hw/rom/gpio/rtc.hpp"
#include "core.hpp"

namespace nba {

namespace core {

Core::Core(std::shared_ptr<Config> config)
    : config(config)
    , cpu(scheduler, bus)
    , irq(cpu, scheduler)
    , dma(bus, irq, scheduler)
    , apu(scheduler, dma, bus, config)
    , ppu(scheduler, irq, dma, config)
    , timer(scheduler, irq, apu)
    , keypad(irq, config)
    , bus(scheduler, {cpu, irq, dma, apu, ppu, timer, keypad}) {
  Reset();
}

void Core::Reset() {
  scheduler.Reset();
  cpu.Reset();
  irq.Reset();
  dma.Reset();
  timer.Reset();
  apu.Reset();
  ppu.Reset();
  bus.Reset();
  keypad.Reset();

  if (config->skip_bios) {
    SkipBootScreen();
  }

  if (config->audio.mp2k_hle_enable) {
    apu.GetMP2K().UseCubicFilter() = config->audio.mp2k_hle_cubic;
    hle_audio_hook = SearchSoundMainRAM();
    if (hle_audio_hook != 0xFFFFFFFF) {
      Log<Info>("Core: detected MP2K audio mixer @ 0x{:08X}", hle_audio_hook);
    }
  } else {
    hle_audio_hook = 0xFFFFFFFF;
  }
}

void Core::Attach(std::vector<u8> const& bios) {
  bus.Attach(bios);
}

void Core::Attach(ROM&& rom) {
  bus.Attach(std::move(rom));
}

auto Core::CreateRTC() -> std::unique_ptr<GPIO> {
  return std::make_unique<RTC>(irq);
}

void Core::Run(int cycles) {
  using HaltControl = Bus::Hardware::HaltControl;

  auto limit = scheduler.GetTimestampNow() + cycles;

  while (scheduler.GetTimestampNow() < limit) {
    if (bus.hw.haltcnt == HaltControl::Halt && irq.HasServableIRQ()) {
      bus.Idle();
      bus.hw.haltcnt = HaltControl::Run;
    }

    if (bus.hw.haltcnt == HaltControl::Run) {
      if (cpu.state.r15 == hle_audio_hook) {
        // TODO: cache the SoundInfo pointer once we have it?
        apu.GetMP2K().SoundMainRAM(
          *bus.GetHostAddress<MP2K::SoundInfo>(
            *bus.GetHostAddress<u32>(0x0300'7FF0)
          )
        );
      }
      cpu.Run();
    } else {
      bus.Step(scheduler.GetRemainingCycleCount());
    }
  }
}

void Core::SkipBootScreen() {
  cpu.SwitchMode(arm::MODE_SYS);
  cpu.state.bank[arm::BANK_SVC][arm::BANK_R13] = 0x03007FE0;
  cpu.state.bank[arm::BANK_IRQ][arm::BANK_R13] = 0x03007FA0;
  cpu.state.r13 = 0x03007F00;
  cpu.state.r15 = 0x08000000;
}

auto Core::SearchSoundMainRAM() -> u32 {
  static constexpr u32 kSoundMainCRC32 = 0x27EA7FCF;
  static constexpr int kSoundMainLength = 48;

  auto& rom = bus.memory.rom.GetRawROM();

  if (rom.size() < kSoundMainLength) {
    return 0xFFFFFFFF;
  }

  u32 address_max = rom.size() - kSoundMainLength;

  for (u32 address = 0; address <= address_max; address += sizeof(u16)) {
    auto crc = crc32(&rom[address], kSoundMainLength);

    if (crc == kSoundMainCRC32) {
      /* We have found SoundMain().
       * The pointer to SoundMainRAM() is stored at offset 0x74.
       */
      address = read<u32>(rom.data(), address + 0x74);
      if (address & 1) {
        address &= ~1;
        address += sizeof(u16) * 2;
      } else {
        address &= ~3;
        address += sizeof(u32) * 2;
      }
      return address;
    }
  }

  return 0xFFFFFFFF;
}

} // namespace nba::core

auto CreateCore(
  std::shared_ptr<Config> config
) -> std::unique_ptr<CoreBase> {
  return std::make_unique<core::Core>(config);
}

} // namespace nba
