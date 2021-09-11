/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <nba/deprecate/config.hpp>
#include <memory>

#include "arm/arm7tdmi.hpp"
#include "bus/bus.hpp"
#include "hw/apu/apu.hpp"
#include "hw/ppu/ppu.hpp"
#include "hw/dma.hpp"
#include "hw/interrupt.hpp"
#include "hw/keypad.hpp"
#include "hw/timer.hpp"
#include "scheduler.hpp"

namespace nba::core {

struct CPU {
  CPU(std::shared_ptr<Config> config);

  void Reset();
  void Run(int cycles);

  std::shared_ptr<Config> config;

  friend struct Bus;

  Scheduler scheduler;
  arm::ARM7TDMI cpu;
  IRQ irq;
  DMA dma;
  APU apu;
  PPU ppu;
  Timer timer;
  KeyPad keypad;
  Bus bus;

  void MP2KSearchSoundMainRAM();
  void MP2KOnSoundMainRAMCalled();

  u32 mp2k_soundmain_address;
};

} // namespace nba::core
