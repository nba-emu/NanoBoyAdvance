/*
 * Copyright (C) 2023 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "arm/arm7tdmi.hpp"
#include "irq.hpp"

namespace nba::core {

void IRQ::LoadState(SaveState const& state) {
  reg_ime = state.irq.reg_ime;
  reg_ie = state.irq.reg_ie;
  reg_if = state.irq.reg_if;
  irq_line = MasterEnable() && HasServableIRQ();
}

void IRQ::CopyState(SaveState& state) {
  state.irq.reg_ime = reg_ime;
  state.irq.reg_ie = reg_ie;
  state.irq.reg_if = reg_if;
}

} // namespace nba::core
