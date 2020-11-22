/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "interrupt.hpp"

namespace nba::core {

auto IRQ::Read(int offset) const -> std::uint8_t {
  switch (offset) {
    case REG_IE|0: return reg_ie & 0xFF;
    case REG_IE|1: return reg_ie >> 8;
    case REG_IF|0: return reg_if & 0xFF;
    case REG_IF|1: return reg_if >> 8;
    case REG_IME:  return reg_ime ? 1 : 0;
  }

  return 0;
}

void IRQ::Write(int offset, std::uint8_t value) {
  switch (offset) {
    case REG_IE|0:
      reg_ie &= 0xFF00;
      reg_ie |= value;
      break;
    case REG_IE|1:
      reg_ie &= 0x00FF;
      reg_ie |= value << 8;
      break;
    case REG_IF|0:
      reg_if &= ~value;
      break;
    case REG_IF|1:
      reg_if &= ~(value << 8);
      break;
    case REG_IME:
      reg_ime = value & 1;
      break;
  }
}

void IRQ::Raise(InterruptSource source, int id) {
  switch (source) {
    case InterruptSource::VBlank:
      reg_if |= 1;
      break;
    case InterruptSource::HBlank:
      reg_if |= 2;
      break;
    case InterruptSource::VCount:
      reg_if |= 4;
      break;
    case InterruptSource::Timer:
      reg_if |= 8 << id;
      break;
    case InterruptSource::Serial:
      reg_if |= 128;
      break;
    case InterruptSource::DMA:
      reg_if |= 256 << id;
      break;
    case InterruptSource::Keypad:
      reg_if |= 4096;
      break;
    case InterruptSource::GamePak:
      reg_if |= 8192;
      break;
  }
}

} // namespace nba::core
