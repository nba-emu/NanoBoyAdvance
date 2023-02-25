/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <arm/arm7tdmi.hpp>

#include "hw/irq/irq.hpp"

namespace nba::core {

/**
 * TODO: figure out what takes priority when an IRQ is asserted
 * at the same time as IE/IF/IME are written.
 */

void IRQ::Reset() {
  reg_ime = 0;
  reg_ie = 0;
  reg_if = 0;
  irq_line = false;
  cpu.IRQLine() = false;
}

auto IRQ::ReadByte(int offset) const -> u8 {
  switch (offset) {
    case REG_IE|0: return reg_ie & 0xFF;
    case REG_IE|1: return reg_ie >> 8;
    case REG_IF|0: return reg_if & 0xFF;
    case REG_IF|1: return reg_if >> 8;
    case REG_IME:  return reg_ime ? 1 : 0;
  }

  return 0;
}

auto IRQ::ReadHalf(int offset) const -> u16 {
  switch (offset) {
    case REG_IE:  return reg_ie;
    case REG_IF:  return reg_if;
    case REG_IME: return reg_ime ? 1 : 0;
  }

  return 0;
}

void IRQ::WriteByte(int offset, u8 value) {
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

  UpdateIRQLine(1);
}

void IRQ::WriteHalf(int offset, u16 value) {
  switch (offset) {
    case REG_IE:
      reg_ie = value & 0x3FFF;
      break;
    case REG_IF:
      reg_if &= ~value;
      break;
    case REG_IME:
      reg_ime = value & 1;
      break;
  }

  UpdateIRQLine(1);
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

  UpdateIRQLine(0);
}

void IRQ::UpdateIRQLine(int event_priority) {
  bool irq_line_new = MasterEnable() && HasServableIRQ();

  if (irq_line != irq_line_new) {
    scheduler.Add(3, Scheduler::EventClass::IRQ_synchronizer_delay, event_priority, (u64)irq_line_new);

    irq_line = irq_line_new;
  }
}

void IRQ::OnIRQDelayPassed(u64 irq_line) {
  cpu.IRQLine() = (bool)irq_line;
}

} // namespace nba::core
