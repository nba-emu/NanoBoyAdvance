/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#pragma once

#include <cstdint>

namespace nba::core {

enum class InterruptSource {
  VBlank,
  HBlank,
  VCount,
  Timer,
  Serial,
  DMA,
  Keypad,
  GamePak
};

class InterruptController {
public:
  InterruptController() { Reset(); }

  void Reset() {
    reg_ime = 0;
    reg_ie = 0;
    reg_if = 0;
  }

  auto Read(int offset) const -> std::uint8_t;
  void Write(int offset, std::uint8_t value);
  void Raise(InterruptSource source, int id = 0);

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

  int reg_ime;
  std::uint16_t reg_ie;
  std::uint16_t reg_if;
};

} // namespace nba::core
