/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <common/log.hpp>
#include <common/punning.hpp>
#include <emulator/cartridge/backup/backup.hpp>
#include <emulator/cartridge/gpio/gpio.hpp>
#include <emulator/cartridge/game_pak.hpp>
#include <nba/deprecate/config.hpp>
#include <memory>
#include <type_traits>

#include "arm/arm7tdmi.hpp"
#include "bus/bus.hpp"
#include "hw/apu/apu.hpp"
#include "hw/ppu/ppu.hpp"
#include "hw/dma.hpp"
#include "hw/interrupt.hpp"
#include "hw/keypad.hpp"
#include "hw/serial.hpp"
#include "hw/timer.hpp"
#include "scheduler.hpp"

namespace nba::core {

struct CPU final : private arm::ARM7TDMI {
  CPU(std::shared_ptr<Config> config);

  void Reset();
  void RunFor(int cycles);

  std::shared_ptr<Config> config;

  struct SystemMemory {
    u8 bios[0x04000];
    u8 wram[0x40000];
    u8 iram[0x08000];
    u32 bios_latch = 0;
  } memory;

  GamePak game_pak;

  struct MMIO {
    u16 rcnt_hack = 0;
  } mmio;

  Scheduler scheduler;
  IRQ irq;
  DMA dma;
  APU apu;
  PPU ppu;
  Timer timer;
  SerialBus serial_bus;
  KeyPad keypad;
  Bus bus;

private:
  friend struct MP2K;
  friend struct Bus;

  template <typename T, bool debug> auto Read(u32 address) -> T {
    return 0;
  }

  void MP2KSearchSoundMainRAM();
  void MP2KOnSoundMainRAMCalled();

  u32 mp2k_soundmain_address;
};

} // namespace nba::core
