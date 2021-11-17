/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <arm/arm7tdmi.hpp>

#include "hw/irq/irq.hpp"

namespace nba::core {

void IRQ::Reset() {
  reg_ime = 0;
  reg_ie = 0;
  reg_if = 0;
  event = nullptr;
  irq_line = false;
  cpu.IRQLine() = false;
}

auto IRQ::Read(int offset) const -> u8 {
  switch (offset) {
    case REG_IE|0: return reg_ie & 0xFF;
    case REG_IE|1: return reg_ie >> 8;
    case REG_IF|0: return reg_if & 0xFF;
    case REG_IF|1: return reg_if >> 8;
    case REG_IME:  return reg_ime ? 1 : 0;
  }

  return 0;
}

void IRQ::Write(int offset, u8 value) {
  switch (offset) {
    case REG_IE|0:
      reg_ie &= 0x3F00;
      reg_ie |= value;
      break;
    case REG_IE|1:
      reg_ie &= 0x00FF;
      reg_ie |= (value << 8) & 0x3F00;
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

  UpdateIRQLine();
}

void IRQ::Raise(IRQ::Source source, int channel) {
  switch (source) {
    case Source::VBlank:
      reg_if |= 1;
      break;
    case Source::HBlank:
      reg_if |= 2;
      break;
    case Source::VCount:
      reg_if |= 4;
      break;
    case Source::Timer:
      reg_if |= 8 << channel;
      break;
    case Source::Serial:
      reg_if |= 128;
      break;
    case Source::DMA:
      reg_if |= 256 << channel;
      break;
    case Source::Keypad:
      reg_if |= 4096;
      break;
    case Source::ROM:
      reg_if |= 8192;
      break;
  }

  UpdateIRQLine();
}

void IRQ::UpdateIRQLine() {
  bool irq_line_new = MasterEnable() && HasServableIRQ();

  if (irq_line != irq_line_new) {
    if (event != nullptr) {
      scheduler.Cancel(event);
    }
    event = scheduler.Add(2, [=](int late) {
      cpu.IRQLine() = irq_line_new;
      event = nullptr;
    });
    irq_line = irq_line_new;
  }
}

} // namespace nba::core
