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
#include "hw/serial.hpp"
#include "hw/timer.hpp"
#include "scheduler.hpp"

namespace nba::core {

struct CPU final : private arm::ARM7TDMI {
  CPU(std::shared_ptr<Config> config);

  void Reset();
  void RunFor(int cycles);

  enum class HaltControl {
    RUN,
    STOP,
    HALT
  };

  std::shared_ptr<Config> config;

  struct SystemMemory {
    u8 bios[0x04000];
    u8 wram[0x40000];
    u8 iram[0x08000];
    u32 bios_latch = 0;
  } memory;

  GamePak game_pak;

  struct MMIO {
    u16 keyinput = 0x3FF;
    u16 rcnt_hack = 0;
    u8 postflg = 0;
    HaltControl haltcnt = HaltControl::RUN;

    struct WaitstateControl {
      int sram = 0;
      int ws0_n = 0;
      int ws0_s = 0;
      int ws1_n = 0;
      int ws1_s = 0;
      int ws2_n = 0;
      int ws2_s = 0;
      int phi = 0;
      int prefetch = 0;
      int cgb = 0;
    } waitcnt;

    struct KeyControl {
      uint16_t input_mask = 0;
      bool interrupt = false;
      bool and_mode = false;
    } keycnt;
  } mmio;

  Scheduler scheduler;
  IRQ irq;
  DMA dma;
  APU apu;
  PPU ppu;
  Timer timer;
  SerialBus serial_bus;
  Bus bus;

private:
  friend struct MP2K;
  friend struct Bus;

  template <typename T, bool debug> auto Read(u32 address) -> T {
    return 0;
  }

  void CheckKeypadInterrupt();
  void OnKeyPress();
  void MP2KSearchSoundMainRAM();
  void MP2KOnSoundMainRAMCalled();

  u32 mp2k_soundmain_address;
};

} // namespace nba::core
