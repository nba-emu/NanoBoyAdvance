/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>
#include <emulator/core/arm/arm7tdmi.hpp>
#include <emulator/core/scheduler.hpp>

namespace nba::core {

class IRQ {
public:
  enum class Source {
    VBlank,
    HBlank,
    VCount,
    Timer,
    Serial,
    DMA,
    Keypad,
    GamePak
  };

  IRQ(arm::ARM7TDMI& cpu, Scheduler& scheduler)
      : cpu(cpu)
      , scheduler(scheduler) {
    Reset();
  }

  void Reset() {
    reg_ime = 0;
    reg_ie = 0;
    reg_if = 0;
    event = nullptr;
    cpu.IRQLine() = false;
  }

  auto Read(int offset) const -> std::uint8_t;
  void Write(int offset, std::uint8_t value);
  void Raise(IRQ::Source source, int channel = 0);

  bool MasterEnable() const {
    return reg_ime != 0;
  }

  bool HasServableIRQ() const {
    return (reg_ie & reg_if) != 0;
  }

private:
  enum Registers {
    REG_IE  = 0,
    REG_IF  = 2,
    REG_IME = 4
  };

  void UpdateIRQLine();

  int reg_ime;
  std::uint16_t reg_ie;
  std::uint16_t reg_if;
  arm::ARM7TDMI& cpu;
  Scheduler& scheduler;
  Scheduler::Event* event = nullptr;
};

} // namespace nba::core
