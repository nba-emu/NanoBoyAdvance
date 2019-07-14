/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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
        (this->*s_opcode_lut_thumb[instruction >> 6])(instruction);
    } else {
        state.r15 &= ~3;

        pipe.opcode[0] = pipe.opcode[1];
        pipe.opcode[1] = ReadWord(state.r15, pipe.fetch_type);
        if (CheckCondition(static_cast<Condition>(instruction >> 28))) {
            int hash = ((instruction >> 16) & 0xFF0) |
                       ((instruction >>  4) & 0x00F);
            (this->*s_opcode_lut_arm[hash])(instruction);
        } else {
            state.r15 += 4;
        }
    }
}

inline void ARM7TDMI::SignalIrq() {
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

inline void ARM7TDMI::SwitchMode(Mode new_mode) {
    auto old_mode = state.cpsr.f.mode;

    if (new_mode == old_mode)
        return;

    state.cpsr.f.mode = new_mode;

    switch (old_mode) {
        case MODE_USR:
        case MODE_SYS:
            state.bank[BANK_NONE][BANK_R13] = state.r13;
            state.bank[BANK_NONE][BANK_R14] = state.r14;
            break;
        case MODE_FIQ:
            state.bank[BANK_FIQ][BANK_R13] = state.r13;
            state.bank[BANK_FIQ][BANK_R14] = state.r14;
            for (int i = 0; i < 5; i++) {
                state.bank[BANK_FIQ][2+i] = state.reg[8+i];
            }
            break;
        case MODE_IRQ:
            state.bank[BANK_IRQ][BANK_R13] = state.r13;
            state.bank[BANK_IRQ][BANK_R14] = state.r14;
            break;
        case MODE_SVC:
            state.bank[BANK_SVC][BANK_R13] = state.r13;
            state.bank[BANK_SVC][BANK_R14] = state.r14;
            break;
        case MODE_ABT:
            state.bank[BANK_ABT][BANK_R13] = state.r13;
            state.bank[BANK_ABT][BANK_R14] = state.r14;
            break;
        case MODE_UND:
            state.bank[BANK_UND][BANK_R13] = state.r13;
            state.bank[BANK_UND][BANK_R14] = state.r14;
            break;
    }

    switch (new_mode) {
        case MODE_USR:
        case MODE_SYS:
            state.r13 = state.bank[BANK_NONE][BANK_R13];
            state.r14 = state.bank[BANK_NONE][BANK_R14];
            p_spsr = &state.spsr[BANK_NONE];
            break;
        case MODE_FIQ:
            state.r13 = state.bank[BANK_FIQ][BANK_R13];
            state.r14 = state.bank[BANK_FIQ][BANK_R14];
            for (int i = 0; i < 5; i++) {
                state.reg[8+i] = state.bank[BANK_FIQ][2+i];
            }
            break;
        case MODE_IRQ:
            state.r13 = state.bank[BANK_IRQ][BANK_R13];
            state.r14 = state.bank[BANK_IRQ][BANK_R14];
            p_spsr = &state.spsr[BANK_IRQ];
            break;
        case MODE_SVC:
            state.r13 = state.bank[BANK_SVC][BANK_R13];
            state.r14 = state.bank[BANK_SVC][BANK_R14];
            p_spsr = &state.spsr[BANK_SVC];
            break;
        case MODE_ABT:
            state.r13 = state.bank[BANK_ABT][BANK_R13];
            state.r14 = state.bank[BANK_ABT][BANK_R14];
            p_spsr = &state.spsr[BANK_ABT];
            break;
        case MODE_UND:
            state.r13 = state.bank[BANK_UND][BANK_R13];
            state.r14 = state.bank[BANK_UND][BANK_R14];
            p_spsr = &state.spsr[BANK_UND];
            break;
    }
}
