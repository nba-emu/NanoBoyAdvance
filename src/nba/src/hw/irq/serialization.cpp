/*
 * Copyright (C) 2025 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "arm/arm7tdmi.hpp"
#include "irq.hpp"

namespace nba::core {

void IRQ::LoadState(SaveState const& state) {
  pending_ime = state.irq.pending_ime;
  pending_ie = state.irq.pending_ie;
  pending_if = state.irq.pending_if;
  
  reg_ime = state.irq.reg_ime;
  reg_ie = state.irq.reg_ie;
  reg_if = state.irq.reg_if;

  irq_line = reg_ime && (reg_ie & reg_if) != 0;

  irq_available = state.irq.irq_available;
}

void IRQ::CopyState(SaveState& state) {
  state.irq.pending_ime = pending_ime;
  state.irq.pending_ie = pending_ie;
  state.irq.pending_if = pending_if;

  state.irq.reg_ime = reg_ime;
  state.irq.reg_ie = reg_ie;
  state.irq.reg_if = reg_if;

  state.irq.irq_available = irq_available;
}

} // namespace nba::core
