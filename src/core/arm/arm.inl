/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

inline void ARM7::Reset() {
    for (int i = 0; i < 16; i++)
        state.reg[i] = 0;

    for (int i = 0; i < BANK_COUNT; i++) {
        for (int j = 0; j < 7; j++)
            state.bank[i][j] = 0;
        state.spsr[i].v = 0;
    }

    state.cpsr.v = 0;
    SwitchMode(MODE_SYS);
    pipe[0] = 0xF0000000;
    pipe[1] = 0xF0000000;
}

inline void ARM7::Run() {
    auto instruction = pipe[0];

    if (state.cpsr.f.thumb) {
        state.r15 &= ~1;

        pipe[0] = pipe[1];
        pipe[1] = ReadHalf(state.r15, ACCESS_SEQ);
        (this->*thumb_lut[instruction >> 6])(instruction);
    } else {
        state.r15 &= ~3;

        pipe[0] = pipe[1];
        pipe[1] = ReadWord(state.r15, ACCESS_SEQ);
        if (CheckCondition(static_cast<Condition>(instruction >> 28))) {
            int hash = ((instruction >> 16) & 0xFF0) |
                       ((instruction >>  4) & 0x00F);
            (this->*arm_lut[hash])(instruction);
        } else {
            state.r15 += 4;
        }
    }
}

inline void ARM7::SignalIrq() {
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
    ARM_ReloadPipeline();
}

inline void ARM7::ARM_ReloadPipeline() {
    pipe[0] = interface->ReadWord(state.r15+0, ACCESS_NSEQ);
    pipe[1] = interface->ReadWord(state.r15+4, ACCESS_SEQ);
    state.r15 += 8;
}

inline void ARM7::Thumb_ReloadPipeline() {
    pipe[0] = interface->ReadHalf(state.r15+0, ACCESS_NSEQ);
    pipe[1] = interface->ReadHalf(state.r15+2, ACCESS_SEQ);
    state.r15 += 4;
}

inline void ARM7::SetNZ(std::uint32_t value) {
    state.cpsr.f.n = value >> 31;
    state.cpsr.f.z = (value == 0);
}

inline void ARM7::BuildConditionTable() {
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
    }
}

inline bool ARM7::CheckCondition(Condition condition) {
    return condition_table[condition][state.cpsr.v >> 28];
}

inline void ARM7::SwitchMode(Mode new_mode) {
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

inline std::uint32_t ARM7::ReadByte(std::uint32_t address, AccessType type) {
    return interface->ReadByte(address, type);
}

inline std::uint32_t ARM7::ReadHalf(std::uint32_t address, AccessType type) {
    return interface->ReadHalf(address & ~1, type);
}

inline std::uint32_t ARM7::ReadWord(std::uint32_t address, AccessType type) {
    return interface->ReadWord(address & ~3, type);
}

inline std::uint32_t ARM7::ReadByteSigned(std::uint32_t address, AccessType type) {
    std::uint32_t value = interface->ReadByte(address, type);

    if (value & 0x80) {
        value |= 0xFFFFFF00;
    }

    return value;
}

inline std::uint32_t ARM7::ReadHalfRotate(std::uint32_t address, AccessType type) {
    auto value = interface->ReadHalf(address & ~1, type);

    if (address & 1) {
        value = (value >> 8) | (value << 24);
    }

    return value;
}

inline std::uint32_t ARM7::ReadHalfSigned(std::uint32_t address, AccessType type) {
    std::uint32_t value;

    if (address & 1) {
        value = interface->ReadByte(address, type);
        if (value & 0x80) {
            value |= 0xFFFFFF00;
        }
    } else {
        value = interface->ReadHalf(address, type);
        if (value & 0x8000) {
            value |= 0xFFFF0000;
        }
    }

    return value;
}

inline std::uint32_t ARM7::ReadWordRotate(std::uint32_t address, AccessType type) {
    auto value = interface->ReadWord(address & ~3, type);
    auto shift = (address & 3) * 8;

    return (value >> shift) | (value << (32 - shift));
}

inline void ARM7::WriteByte(std::uint32_t address, std::uint8_t  value, AccessType type) {
    interface->WriteByte(address, value, type);
}

inline void ARM7::WriteHalf(std::uint32_t address, std::uint16_t value, AccessType type) {
    interface->WriteHalf(address & ~1, value, type);
}

inline void ARM7::WriteWord(std::uint32_t address, std::uint32_t value, AccessType type) {
    interface->WriteWord(address & ~3, value, type);
}
