/*
 * Copyright (C) 2024 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include "arm/arm7tdmi.hpp"

namespace nba::core::arm {

void ARM7TDMI::LoadState(SaveState const& save_state) {
  for(int i = 0; i < 16; i++) {
    state.reg[i] = save_state.arm.regs.gpr[i];
  }

  for(int i = 0; i < BANK_COUNT; i++) {
    for(int j = 0; j < 7; j++) {
      state.bank[i][j] = save_state.arm.regs.bank[i][j];
    }
    state.spsr[i].v = save_state.arm.regs.spsr[i];
  }

  state.cpsr.v = save_state.arm.regs.cpsr;

  auto bank = GetRegisterBankByMode(state.cpsr.f.mode);
  if(bank != BANK_NONE) {
    p_spsr = &state.spsr[bank];
  } else {
    p_spsr = &state.cpsr;
  }

  pipe.access = save_state.arm.pipe.access;

  for(int i = 0; i < 2; i++) {
    pipe.opcode[i] = save_state.arm.pipe.opcode[i];
  }

  irq_line = save_state.arm.irq_line;

  // TODO: save and restore these variables:
  ldm_usermode_conflict = false;
  cpu_mode_is_invalid = false;
  latch_irq_disable = state.cpsr.f.mask_irq;
}

void ARM7TDMI::CopyState(SaveState& save_state) {
  for(int i = 0; i < 16; i++) {
    save_state.arm.regs.gpr[i] = state.reg[i];
  }

  for(int i = 0; i < BANK_COUNT; i++) {
    for(int j = 0; j < 7; j++) {
      save_state.arm.regs.bank[i][j] = state.bank[i][j];
    }
    save_state.arm.regs.spsr[i] = state.spsr[i].v;
  }

  save_state.arm.regs.cpsr = state.cpsr.v;

  save_state.arm.pipe.access = pipe.access;

  for(int i = 0; i < 2; i++) {
    save_state.arm.pipe.opcode[i] = pipe.opcode[i];
  }

  save_state.arm.irq_line = irq_line;
}

} // namespace nba::core::arm
