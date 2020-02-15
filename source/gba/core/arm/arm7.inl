/*
 * Copyright (C) 2020 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

inline void ARM7TDMI::Reset() {
  state.Reset();
  
  SwitchMode(MODE_SYS);
  
  pipe.opcode[0] = 0xF0000000;
  pipe.opcode[1] = 0xF0000000;
  pipe.fetch_type = ACCESS_NSEQ;
}

inline void ARM7TDMI::Run() {
  auto instruction = pipe.opcode[0];

  if (state.cpsr.f.thumb) {
    state.r15 &= ~1;

    pipe.opcode[0] = pipe.opcode[1];
    pipe.opcode[1] = ReadHalf(state.r15, pipe.fetch_type);
    (this->*opcode_lut_16[instruction >> 6])(instruction);
  } else {
    state.r15 &= ~3;

    pipe.opcode[0] = pipe.opcode[1];
    pipe.opcode[1] = ReadWord(state.r15, pipe.fetch_type);
    if (CheckCondition(static_cast<Condition>(instruction >> 28))) {
      int hash = ((instruction >> 16) & 0xFF0) |
                 ((instruction >>  4) & 0x00F);
      (this->*opcode_lut_32[hash])(instruction);
    } else {
      state.r15 += 4;
    }
  }
}

inline void ARM7TDMI::SignalIRQ() {
  if (state.cpsr.f.mask_irq) {
    return;
  }

  if (state.cpsr.f.thumb) {
    /* Store return address in r14<irq>. */
    state.bank[BANK_IRQ][BANK_R14] = state.r15;

    /* Save program status and switch to IRQ mode. */
    state.spsr[BANK_IRQ].v = state.cpsr.v;
    SwitchMode(MODE_IRQ);
    state.cpsr.f.thumb = 0;
    state.cpsr.f.mask_irq = 1;
  } else {
    /* Store return address in r14<irq>. */
    state.bank[BANK_IRQ][BANK_R14] = state.r15 - 4;

    /* Save program status and switch to IRQ mode. */
    state.spsr[BANK_IRQ].v = state.cpsr.v;
    SwitchMode(MODE_IRQ);
    state.cpsr.f.mask_irq = 1;
  }

  /* Jump to exception vector. */
  state.r15 = 0x18;
  ReloadPipeline32();
}

inline void ARM7TDMI::ReloadPipeline32() {
  pipe.opcode[0] = interface->ReadWord(state.r15+0, ACCESS_NSEQ);
  pipe.opcode[1] = interface->ReadWord(state.r15+4, ACCESS_SEQ);
  pipe.fetch_type = ACCESS_SEQ;
  state.r15 += 8;
}

inline void ARM7TDMI::ReloadPipeline16() {
  pipe.opcode[0] = interface->ReadHalf(state.r15+0, ACCESS_NSEQ);
  pipe.opcode[1] = interface->ReadHalf(state.r15+2, ACCESS_SEQ);
  pipe.fetch_type = ACCESS_SEQ;
  state.r15 += 4;
}

inline void ARM7TDMI::BuildConditionTable() {
  for (int flags = 0; flags < 16; flags++) {
    bool n = flags & 8;
    bool z = flags & 4;
    bool c = flags & 2;
    bool v = flags & 1;

    condition_table[COND_EQ][flags] = z;
    condition_table[COND_NE][flags] = !z;
    condition_table[COND_CS][flags] =  c;
    condition_table[COND_CC][flags] = !c;
    condition_table[COND_MI][flags] =  n;
    condition_table[COND_PL][flags] = !n;
    condition_table[COND_VS][flags] =  v;
    condition_table[COND_VC][flags] = !v;
    condition_table[COND_HI][flags] =  c && !z;
    condition_table[COND_LS][flags] = !c ||  z;
    condition_table[COND_GE][flags] = n == v;
    condition_table[COND_LT][flags] = n != v;
    condition_table[COND_GT][flags] = !(z || (n != v));
    condition_table[COND_LE][flags] =  (z || (n != v));
    condition_table[COND_AL][flags] = true;
    condition_table[COND_NV][flags] = false;
  }
}

inline bool ARM7TDMI::CheckCondition(Condition condition) {
  if (condition == COND_AL)
    return true;
  return condition_table[condition][state.cpsr.v >> 28];
}

inline auto ARM7TDMI::GetRegisterBankByMode(Mode mode) -> Bank {
  /* TODO: reverse-engineer which bank the CPU defaults to for invalid modes. */
  switch (mode) {
    case MODE_USR:
    case MODE_SYS:
      return BANK_NONE;
    case MODE_FIQ:
      return BANK_FIQ;
    case MODE_IRQ:
      return BANK_IRQ;
    case MODE_SVC:
      return BANK_SVC;
    case MODE_ABT:
      return BANK_ABT;
    case MODE_UND:
      return BANK_UND;
  }

  return BANK_UND;
}

inline void ARM7TDMI::SwitchMode(Mode new_mode) {
  auto old_bank = GetRegisterBankByMode(state.cpsr.f.mode);
  auto new_bank = GetRegisterBankByMode(new_mode);

  state.cpsr.f.mode = new_mode; 

  if (new_bank != BANK_NONE) {
    p_spsr = &state.spsr[new_bank]; 
  } else {
    /* In system/user mode reading from SPSR returns the current CPSR value.
     * However writes to SPSR appear to do nothing.
     * We take care of this fact in the MSR implementation.
     */
    p_spsr = &state.cpsr;
  }

  if (old_bank == new_bank) {
    return;
  }

  if (old_bank == BANK_FIQ || new_bank == BANK_FIQ) {
    for (int i = 0; i < 7; i++){
      state.bank[old_bank][i] = state.reg[8+i];
    }

    for (int i = 0; i < 7; i++) {
      state.reg[8+i] = state.bank[new_bank][i];
    }
  } else {
    state.bank[old_bank][5] = state.r13;
    state.bank[old_bank][6] = state.r14;

    state.r13 = state.bank[new_bank][5];
    state.r14 = state.bank[new_bank][6];
  }
}
