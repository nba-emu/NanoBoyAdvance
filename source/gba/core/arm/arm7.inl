/**
  * Copyright (C) 2019 fleroviux (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

template <typename Tinterface>
inline void ARM7TDMI<Tinterface>::Reset() {
  state.Reset();
  
  SwitchMode(MODE_SYS);
  
  pipe.opcode[0] = 0xF0000000;
  pipe.opcode[1] = 0xF0000000;
  pipe.fetch_type = ACCESS_NSEQ;
}

template <typename Tinterface>
inline void ARM7TDMI<Tinterface>::Run() {
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

template <typename Tinterface>
inline void ARM7TDMI<Tinterface>::SignalIRQ() {
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

template <typename Tinterface>
inline void ARM7TDMI<Tinterface>::ReloadPipeline32() {
  pipe.opcode[0] = interface->ReadWord(state.r15+0, ACCESS_NSEQ);
  pipe.opcode[1] = interface->ReadWord(state.r15+4, ACCESS_SEQ);
  pipe.fetch_type = ACCESS_SEQ;
  state.r15 += 8;
}

template <typename Tinterface>
inline void ARM7TDMI<Tinterface>::ReloadPipeline16() {
  pipe.opcode[0] = interface->ReadHalf(state.r15+0, ACCESS_NSEQ);
  pipe.opcode[1] = interface->ReadHalf(state.r15+2, ACCESS_SEQ);
  pipe.fetch_type = ACCESS_SEQ;
  state.r15 += 4;
}

template <typename Tinterface>
inline void ARM7TDMI<Tinterface>::BuildConditionTable() {
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

template <typename Tinterface>
inline bool ARM7TDMI<Tinterface>::CheckCondition(Condition condition) {
  if (condition == COND_AL)
    return true;
  return condition_table[condition][state.cpsr.v >> 28];
}

template <typename Tinterface>
inline auto ARM7TDMI<Tinterface>::GetRegisterBankByMode(Mode mode) -> Bank {
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
}

template <typename Tinterface>
inline void ARM7TDMI<Tinterface>::SwitchMode(Mode new_mode) {
  auto old_bank = GetRegisterBankByMode(state.cpsr.f.mode);
  auto new_bank = GetRegisterBankByMode(new_mode);

  state.cpsr.f.mode = new_mode; 
  p_spsr = &state.spsr[new_bank];

  if (old_bank == new_bank)
    return;

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
