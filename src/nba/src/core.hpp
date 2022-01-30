/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <nba/core.hpp>

#include "arm/arm7tdmi.hpp"
#include "bus/bus.hpp"
#include "hw/apu/apu.hpp"
#include "hw/ppu/ppu.hpp"
#include "hw/dma/dma.hpp"
#include "hw/irq/irq.hpp"
#include "hw/keypad/keypad.hpp"
#include "hw/timer/timer.hpp"
#include "cheat_device.hpp"
#include "scheduler.hpp"

namespace nba::core {

struct Core final : CoreBase {
  Core(std::shared_ptr<Config> config);

  void Reset() override;
  void Attach(std::vector<u8> const& bios) override;
  void Attach(ROM&& rom) override;
  auto CreateRTC() -> std::unique_ptr<GPIO> override;
  void Run(int cycles) override;

private:
  void SkipBootScreen();
  auto SearchSoundMainRAM() -> u32;

  u32 hle_audio_hook;
  std::shared_ptr<Config> config;

  Scheduler scheduler;

  arm::ARM7TDMI cpu;
  IRQ irq;
  DMA dma;
  APU apu;
  PPU ppu;
  Timer timer;
  KeyPad keypad;
  Bus bus;
  CheatDevice cheat_device;
};

} // namespace nba::core
