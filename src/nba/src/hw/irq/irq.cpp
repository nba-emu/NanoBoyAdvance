/*
 * Copyright (C) 2024 fleroviux
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

IRQ::IRQ(arm::ARM7TDMI& cpu, Scheduler& scheduler)
    : cpu(cpu)
    , scheduler(scheduler) {
  scheduler.Register(Scheduler::EventClass::IRQ_write_io, this, &IRQ::OnWriteIO);
  scheduler.Register(Scheduler::EventClass::IRQ_update_ie_and_if, this, &IRQ::UpdateIEAndIF);
  scheduler.Register(Scheduler::EventClass::IRQ_update_irq_line, this, &IRQ::UpdateIRQLine);

  Reset();
}

void IRQ::Reset() {
  pending_ime = 0;
  pending_ie = 0;
  pending_if = 0;

  reg_ime = 0;
  reg_ie = 0;
  reg_if = 0;

  irq_line = false;
  cpu.IRQLine() = false;

  irq_available = false;
}

auto IRQ::ReadByte(int offset) const -> u8 {
  switch(offset) {
    case REG_IE|0: return reg_ie & 0xFF;
    case REG_IE|1: return reg_ie >> 8;
    case REG_IF|0: return reg_if & 0xFF;
    case REG_IF|1: return reg_if >> 8;
    case REG_IME:  return reg_ime ? 1 : 0;
  }

  return 0;
}

auto IRQ::ReadHalf(int offset) const -> u16 {
  switch(offset) {
    case REG_IE:  return reg_ie;
    case REG_IF:  return reg_if;
    case REG_IME: return reg_ime ? 1 : 0;
  }

  return 0;
}

void IRQ::WriteByte(int offset, u8 value) {
  switch(offset) {
    case REG_IE|0:
      pending_ie &= 0x3F00;
      pending_ie |= value;
      break;
    case REG_IE|1:
      pending_ie &= 0x00FF;
      pending_ie |= (value << 8) & 0x3F00;
      break;
    case REG_IF|0:
      pending_if &= ~value;
      break;
    case REG_IF|1:
      pending_if &= ~(value << 8);
      break;
    case REG_IME:
      pending_ime = value & 1;
      break;
  }

  scheduler.Add(1, Scheduler::EventClass::IRQ_write_io, 1);
}

void IRQ::WriteHalf(int offset, u16 value) {
  switch(offset) {
    case REG_IE:
      pending_ie = value & 0x3FFF;
      break;
    case REG_IF:
      pending_if &= ~value;
      break;
    case REG_IME:
      pending_ime = value & 1;
      break;
  }

  scheduler.Add(1, Scheduler::EventClass::IRQ_write_io, 1);
}

void IRQ::Raise(IRQ::Source source, int channel) {
  switch(source) {
    case Source::VBlank:
      pending_if |= 1;
      break;
    case Source::HBlank:
      pending_if |= 2;
      break;
    case Source::VCount:
      pending_if |= 4;
      break;
    case Source::Timer:
      pending_if |= 8 << channel;
      break;
    case Source::Serial:
      pending_if |= 128;
      break;
    case Source::DMA:
      pending_if |= 256 << channel;
      break;
    case Source::Keypad:
      pending_if |= 4096;
      break;
    case Source::ROM:
      pending_if |= 8192;
      break;
  }

  scheduler.Add(1, Scheduler::EventClass::IRQ_write_io, 0);
}

void IRQ::OnWriteIO() {
  reg_ime = pending_ime;
  reg_ie = pending_ie;
  reg_if = pending_if;

  const bool irq_available_new = reg_ie & reg_if;
  
  if(irq_available != irq_available_new) {
    scheduler.Add(1, Scheduler::EventClass::IRQ_update_ie_and_if, 0, (u64)irq_available_new);
  }

  const bool irq_line_new = reg_ime && irq_available_new;

  if(irq_line != irq_line_new) {
    scheduler.Add(2, Scheduler::EventClass::IRQ_update_irq_line, 0, (u64)irq_line_new);

    irq_line = irq_line_new;
  }
}

void IRQ::UpdateIEAndIF(u64 irq_available) {
  this->irq_available = (bool)irq_available;
}

void IRQ::UpdateIRQLine(u64 irq_line) {
  cpu.IRQLine() = (bool)irq_line;
}

} // namespace nba::core
