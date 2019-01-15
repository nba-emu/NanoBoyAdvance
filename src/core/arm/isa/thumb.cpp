/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../arm.hpp"

using namespace ARM;

std::array<ARM7::ThumbInstruction, 1024> ARM7::thumb_lut = MakeThumbLut();

template <int op, int imm>
void ARM7::Thumb_MoveShiftedRegister(std::uint16_t instruction) {
    // THUMB.1 Move shifted register
    int dst   = (instruction >> 0) & 7;
    int src   = (instruction >> 3) & 7;
    int carry = state.cpsr.f.c;

    std::uint32_t result = state.reg[src];

    DoShift(op, result, imm, carry, true);

    /* Update flags */
    state.cpsr.f.c = carry;
    state.cpsr.f.z = (result == 0);
    state.cpsr.f.n = result >> 31;

    state.reg[dst] = result;
    state.r15 += 2;
}

template <bool immediate, bool subtract, int field3>
void ARM7::Thumb_AddSub(std::uint16_t instruction) {
    int dst = (instruction >> 0) & 7;
    int src = (instruction >> 3) & 7;
    std::uint32_t operand = immediate ? field3 : state.reg[field3];

    if (subtract) {
        state.reg[dst] = SUB(state.reg[src], operand, true);
    } else {
        state.reg[dst] = ADD(state.reg[src], operand, true);
    }

    state.r15 += 2;
}

template <int op, int dst>
void ARM7::Thumb_Op3(std::uint16_t instruction) {
    // THUMB.3 Move/compare/add/subtract immediate
    std::uint32_t imm = instruction & 0xFF;

    switch (op) {
    case 0b00:
        /* MOV rD, #imm */
        state.reg[dst] = imm;
        state.cpsr.f.n = 0;
        state.cpsr.f.z = imm == 0;
        break;
    case 0b01:
        /* CMP rD, #imm */
        SUB(state.reg[dst], imm, true);
        break;
    case 0b10:
        /* ADD rD, #imm */
        state.reg[dst] = ADD(state.reg[dst], imm, true);
        break;
    case 0b11:
        /* SUB rD, #imm */
        state.reg[dst] = SUB(state.reg[dst], imm, true);
        break;
    }

    state.r15 += 2;
}

template <int op>
void ARM7::Thumb_ALU(std::uint16_t instruction) {
    int dst = (instruction >> 0) & 7;
    int src = (instruction >> 3) & 7;

    /* Order of opcodes has been rearranged for readabilities sake. */
    switch (static_cast<ThumbDataOp>(op)) {

    case ThumbDataOp::MVN:
        state.reg[dst] = ~state.reg[src];
        SetNZ(state.reg[dst]);
        break;

    /* Bitwise logical */
    case ThumbDataOp::AND:
        state.reg[dst] &= state.reg[src];
        SetNZ(state.reg[dst]);
        break;
    case ThumbDataOp::BIC:
        state.reg[dst] &= ~state.reg[src];
        SetNZ(state.reg[dst]);
        break;
    case ThumbDataOp::ORR:
        state.reg[dst] |= state.reg[src];
        SetNZ(state.reg[dst]);
        break;
    case ThumbDataOp::EOR:
        state.reg[dst] ^= state.reg[src];
        SetNZ(state.reg[dst]);
        break;
    case ThumbDataOp::TST: {
        std::uint32_t result = state.reg[dst] & state.reg[src];
        SetNZ(result);
        break;
    }

    /* LSL, LSR, ASR, ROR */
    case ThumbDataOp::LSL: {
        int carry = state.cpsr.f.c;
        LSL(state.reg[dst], state.reg[src], carry);
        SetNZ(state.reg[dst]);
        state.cpsr.f.c = carry;
        break;
    }
    case ThumbDataOp::LSR: {
        int carry = state.cpsr.f.c;
        LSR(state.reg[dst], state.reg[src], carry, false);
        SetNZ(state.reg[dst]);
        state.cpsr.f.c = carry;
        break;
    }
    case ThumbDataOp::ASR: {
        int carry = state.cpsr.f.c;
        ASR(state.reg[dst], state.reg[src], carry, false);
        SetNZ(state.reg[dst]);
        state.cpsr.f.c = carry;
        break;
    }
    case ThumbDataOp::ROR: {
        int carry = state.cpsr.f.c;
        ROR(state.reg[dst], state.reg[src], carry, false);
        SetNZ(state.reg[dst]);
        state.cpsr.f.c = carry;
        break;
    }

    /* Add/Sub with carry, NEG, comparison, multiply */
    case ThumbDataOp::ADC:
        state.reg[dst] = ADC(state.reg[dst], state.reg[src], true);
        break;
    case ThumbDataOp::SBC:
        state.reg[dst] = SBC(state.reg[dst], state.reg[src], true);
        break;
    case ThumbDataOp::NEG:
        state.reg[dst] = SUB(0, state.reg[src], true);
        break;
    case ThumbDataOp::CMP:
        SUB(state.reg[dst], state.reg[src], true);
        break;
    case ThumbDataOp::CMN:
        ADD(state.reg[dst], state.reg[src], true);
        break;
    case ThumbDataOp::MUL:
        /* TODO: calculate internal cycles. */
        state.reg[dst] *= state.reg[src];
        SetNZ(state.reg[dst]);
        state.cpsr.f.c = 0;
        break;

    }

    state.r15 += 2;
}

template <int op, bool high1, bool high2>
void ARM7::Thumb_HighRegisterOps_BX(std::uint16_t instruction) {
    // THUMB.5 Hi register operations/branch exchange
    int dst = (instruction >> 0) & 7;
    int src = (instruction >> 3) & 7;

    // Instruction may access higher registers r8 - r15 ("Hi register").
    // This is archieved using two extra bits that displace the register number by 8.
    if (high1) dst |= 8;
    if (high2) src |= 8;

    std::uint32_t operand = state.reg[src];

    if (src == 15) operand &= ~1;

    /* Check for Branch & Exchange (bx) instruction. */
    if (op == 3) {
        /* LSB indicates new instruction set (0 = ARM, 1 = THUMB) */
        if (operand & 1) {
            state.r15 = operand & ~1;
            Thumb_ReloadPipeline();
        } else {
            state.cpsr.f.thumb = 0;
            state.r15 = operand & ~3;
            ARM_ReloadPipeline();
        }
    /* Check for Compare (CMP) instruction. */
    } else if (op == 1) {
        SUB(state.reg[dst], operand, true);
        state.r15 += 2;
    /* Otherwise instruction is ADD or MOv. */
    } else {
        if (op == 0) state.reg[dst] += operand;
        if (op == 2) state.reg[dst]  = operand;

        if (dst == 15) {
            state.r15 &= ~1;
            Thumb_ReloadPipeline();
        } else {
            state.r15 += 2;
        }
    }
}

template <int dst>
void ARM7::Thumb_LoadStoreRelativePC(std::uint16_t instruction) {
    std::uint32_t offset  = instruction & 0xFF;
    std::uint32_t address = (state.r15 & ~2) + (offset << 2);

    state.reg[dst] = ReadWord(address, ACCESS_NSEQ);
    state.r15 += 2;
}

template <int op, int off>
void ARM7::Thumb_LoadStoreOffsetReg(std::uint16_t instruction) {
    int dst  = (instruction >> 0) & 7;
    int base = (instruction >> 3) & 7;

    std::uint32_t address = state.reg[base] + state.reg[off];

    switch (op) {
    case 0b00: // STR
        WriteWord(address, state.reg[dst], ACCESS_NSEQ);
        break;
    case 0b01: // STRB
        WriteByte(address, (std::uint8_t)state.reg[dst], ACCESS_NSEQ);
        break;
    case 0b10: // LDR
        state.reg[dst] = ReadWordRotate(address, ACCESS_NSEQ);
        break;
    case 0b11: // LDRB
        state.reg[dst] = ReadByte(address, ACCESS_NSEQ);
        break;
    }

    state.r15 += 2;
}

template <int op, int off>
void ARM7::Thumb_LoadStoreSigned(std::uint16_t instruction) {
    int dst  = (instruction >> 0) & 7;
    int base = (instruction >> 3) & 7;
    
    std::uint32_t address = state.reg[base] + state.reg[off];

    switch (op) {
    case 0b00:
        /* STRH rD, [rB, rO] */
        WriteHalf(address, state.reg[dst], ACCESS_NSEQ);
        break;
    case 0b01:
        /* LDSB rD, [rB, rO] */
        state.reg[dst] = ReadByteSigned(address, ACCESS_NSEQ);
        break;
    case 0b10:
        /* LDRH rD, [rB, rO] */
        state.reg[dst] = ReadHalfRotate(address, ACCESS_NSEQ);
        break;
    case 0b11:
        /* LDSH rD, [rB, rO] */
        state.reg[dst] = ReadHalfSigned(address, ACCESS_NSEQ);
        break;
    }

    state.r15 += 2;
}

template <int op, int imm>
void ARM7::Thumb_LoadStoreOffsetImm(std::uint16_t instruction) {
    int dst  = (instruction >> 0) & 7;
    int base = (instruction >> 3) & 7;

    switch (op) {
    case 0b00:
        /* STR rD, [rB, #imm] */
        WriteWord(state.reg[base] + imm * 4, state.reg[dst], ACCESS_NSEQ);
        break;
    case 0b01:
        /* LDR rD, [rB, #imm] */
        state.reg[dst] = ReadWordRotate(state.reg[base] + imm * 4, ACCESS_NSEQ);
        break;
    case 0b10:
        /* STRB rD, [rB, #imm] */
        WriteByte(state.reg[base] + imm, state.reg[dst], ACCESS_NSEQ);
        break;
    case 0b11:
        /* LDRB rD, [rB, #imm] */
        state.reg[dst] = ReadByte(state.reg[base] + imm, ACCESS_NSEQ);
        break;
    }

    state.r15 += 2;
}

template <bool load, int imm>
void ARM7::Thumb_LoadStoreHword(std::uint16_t instruction) {
    int dst  = (instruction >> 0) & 7;
    int base = (instruction >> 3) & 7;

    std::uint32_t address = state.reg[base] + imm * 2;

    if (load) {
        state.reg[dst] = ReadHalfRotate(address, ACCESS_NSEQ);
    } else {
        WriteHalf(address, state.reg[dst], ACCESS_NSEQ);
    }

    state.r15 += 2;
}

template <bool load, int dst>
void ARM7::Thumb_LoadStoreRelativeToSP(std::uint16_t instruction) {
    std::uint32_t offset  = instruction & 0xFF;
    std::uint32_t address = state.reg[13] + offset * 4;

    if (load) {
        state.reg[dst] = ReadWordRotate(address, ACCESS_NSEQ);
    } else {
        WriteWord(address, state.reg[dst], ACCESS_NSEQ);
    }

    state.r15 += 2;
}

template <bool stackptr, int dst>
void ARM7::Thumb_LoadAddress(std::uint16_t instruction) {
    std::uint32_t offset = (instruction  & 0xFF) << 2;

    if (stackptr) {
        state.reg[dst] = state.r13 + offset;
    } else {
        state.reg[dst] = (state.r15 & ~2) + offset;
    }

    state.r15 += 2;
}

template <bool sub>
void ARM7::Thumb_AddOffsetToSP(std::uint16_t instruction) {
    std::uint32_t offset = (instruction  & 0x7F) * 4;

    state.r13 += sub ? -offset : offset;
    state.r15 += 2;
}

template <bool pop, bool rbit>
void ARM7::Thumb_PushPop(std::uint16_t instruction) {
    std::uint32_t address = state.r13;

    /* TODO: Empty register list, figure out timings. */
    if (pop) {
        for (int reg = 0; reg <= 7; reg++) {
            if (instruction & (1<<reg)) {
                state.reg[reg] = ReadWord(address, ACCESS_SEQ);
                address += 4;
            }
        }
        if (rbit) {
            state.reg[15] = ReadWord(address, ACCESS_SEQ) & ~1;
            state.reg[13] = address + 4;
            Thumb_ReloadPipeline();
            return;
        }
        state.r13 = address;
    } else {
        /* Calculate internal start address (final r13 value) */
        for (int reg = 0; reg <= 7; reg++) {
            if (instruction & (1 << reg))
                address -= 4;
        }
        if (rbit) address -= 4;

        /* Store address in r13 before we mess with it. */
        state.r13 = address;

        for (int reg = 0; reg <= 7; reg++) {
            if (instruction & (1<<reg)) {
                WriteWord(address, state.reg[reg], ACCESS_SEQ);
                address += 4;
            }
        }
        if (rbit) {
            WriteWord(address, state.r14, ACCESS_SEQ);
        }
    }

    state.r15 += 2;
}

template <bool load, int base>
void ARM7::Thumb_LoadStoreMultiple(std::uint16_t instruction) {
    /* TODO: handle empty register list, work on timings. */
    if (load) {
        std::uint32_t address = state.reg[base];

        for (int i = 0; i <= 7; i++) {
            if (instruction & (1<<i)) {
                state.reg[i] = ReadWord(address, ACCESS_SEQ);
                address += 4;
            }
        }
        if (~instruction & (1<<base)) {
            state.reg[base] = address;
        }
    } else {
        int reg = 0;

        /* First Loop - Run to first register (nonsequential access) */
        for (; reg <= 7; reg++) {
            if (instruction & (1<<reg)) {
                WriteWord(state.reg[base], state.reg[reg], ACCESS_NSEQ);
                state.reg[base] += 4;
                break;
            }
        }
        reg++;
        /* Second Loop - Run until end (sequential accesses) */
        for (; reg <= 7; reg++) {
            if (instruction & (1 << reg)) {
                WriteWord(state.reg[base], state.reg[reg], ACCESS_SEQ);
                state.reg[base] += 4;
            }
        }
    }

    state.r15 += 2;
}

template <int cond>
void ARM7::Thumb_ConditionalBranch(std::uint16_t instruction) {
    if (CheckCondition(static_cast<Condition>(cond))) {
        std::uint32_t imm = instruction & 0xFF;

        /* Sign-extend immediate value. */
        if (imm & 0x80) {
            imm |= 0xFFFFFF00;
        }

        state.r15 += imm * 2;
        Thumb_ReloadPipeline();
    } else {
        state.r15 += 2;
    }
}

void ARM7::Thumb_SWI(std::uint16_t instruction) {
    /* Save return address and program status. */
    state.bank[BANK_SVC][BANK_R14] = state.r15 - 2;
    state.spsr[BANK_SVC].v = state.cpsr.v;

    /* Switch to SVC mode and disable interrupts. */
    SwitchMode(MODE_SVC);
    state.cpsr.f.thumb = 0;
    state.cpsr.f.mask_irq = 1;

    /* Jump to execution vector */
    state.r15 = 0x08;
    ARM_ReloadPipeline();
}

void ARM7::Thumb_UnconditionalBranch(std::uint16_t instruction) {
    std::uint32_t imm = (instruction & 0x3FF) * 2;

    /* Sign-extend immediate value. */
    if (instruction & 0x400) {
        imm |= 0xFFFFF800;
    }

    state.r15 += imm;
    Thumb_ReloadPipeline();
}

template <bool second_instruction>
void ARM7::Thumb_LongBranchLink(std::uint16_t instruction) {
    std::uint32_t imm = instruction & 0x7FF;

    if (!second_instruction) {
        imm <<= 12;
        if (imm & 0x400000) {
            imm |= 0xFF800000;
        }
        state.r14 = state.r15 + imm;
        state.r15 += 2;
    } else {
        std::uint32_t temp = state.r15 - 2;

        state.r15 = (state.r14 + imm * 2) & ~1;
        state.r14 = temp | 1;
        Thumb_ReloadPipeline();
    }
}

/* This should never get called. */
void ARM7::Thumb_Undefined(std::uint16_t instruction) { }

template <std::uint16_t instruction>
constexpr ARM7::ThumbInstruction ARM7::GetThumbHandler() {
    if ((instruction & 0xF800) < 0x1800) {
        // THUMB.1 Move shifted register
        return &ARM7::Thumb_MoveShiftedRegister<(instruction >> 11) & 3,     /* Operation */
                               (instruction >>  6) & 0x1F>; /* Offset5 */
    }
    if ((instruction & 0xF800) == 0x1800) {
        // THUMB.2 Add/subtract
        return &ARM7::Thumb_AddSub<(instruction >> 10) & 1,  /* Immediate-Flag */
                               (instruction >>  9) & 1,  /* Subtract-Flag */
                               (instruction >>  6) & 7>; /* 3-bit field */
    }
    if ((instruction & 0xE000) == 0x2000) {
        // THUMB.3 Move/compare/add/subtract immediate
        return &ARM7::Thumb_Op3<(instruction >> 11) & 3,   /* Operation */
                               (instruction >>  8) & 7>;  /* rD */
    }
    if ((instruction & 0xFC00) == 0x4000) {
        // THUMB.4 ALU operations
        return &ARM7::Thumb_ALU<(instruction >> 6) & 0xF>; /* Operation */
    }
    if ((instruction & 0xFC00) == 0x4400) {
        // THUMB.5 Hi register operations/branch exchange
        return &ARM7::Thumb_HighRegisterOps_BX<(instruction >> 8) & 3,  /* Operation */
                               (instruction >> 7) & 1,  /* High 1 */
                               (instruction >> 6) & 1>; /* High 2 */
    }
    if ((instruction & 0xF800) == 0x4800) {
        // THUMB.6 PC-relative load
        return &ARM7::Thumb_LoadStoreRelativePC<(instruction >>  8) & 7>; /* rD */
    }
    if ((instruction & 0xF200) == 0x5000) {
        // THUMB.7 Load/store with register offset
        return &ARM7::Thumb_LoadStoreOffsetReg<(instruction >> 10) & 3,  /* Operation (LB) */
                               (instruction >>  6) & 7>; /* rO */
    }
    if ((instruction & 0xF200) == 0x5200) {
        // THUMB.8 Load/store sign-extended byte/halfword
        return &ARM7::Thumb_LoadStoreSigned<(instruction >> 10) & 3,  /* Operation (HS) */
                               (instruction >>  6) & 7>; /* rO */
    }
    if ((instruction & 0xE000) == 0x6000) {
        // THUMB.9 Load store with immediate offset
        return &ARM7::Thumb_LoadStoreOffsetImm<(instruction >> 11) & 3,     /* Operation (BL) */
                               (instruction >>  6) & 0x1F>; /* Offset5 */
    }
    if ((instruction & 0xF000) == 0x8000) {
        // THUMB.10 Load/store halfword
        return &ARM7::Thumb_LoadStoreHword<(instruction >> 11) & 1,     /* Load-Flag */
                                (instruction >>  6) & 0x1F>; /* Offset5 */
    }
    if ((instruction & 0xF000) == 0x9000) {
        // THUMB.11 SP-relative load/store
        return &ARM7::Thumb_LoadStoreRelativeToSP<(instruction >> 11) & 1,  /* Load-Flag */
                                (instruction >>  8) & 7>; /* rD */
    }
    if ((instruction & 0xF000) == 0xA000) {
        // THUMB.12 Load address
        return &ARM7::Thumb_LoadAddress<(instruction >> 11) & 1,  /* 0=PC??? 1=SP */
                                (instruction >>  8) & 7>; /* rD */
    }
    if ((instruction & 0xFF00) == 0xB000) {
        // THUMB.13 Add offset to stack pointer
        return &ARM7::Thumb_AddOffsetToSP<(instruction >> 7) & 1>; /* Subtract-Flag */
    }
    if ((instruction & 0xF600) == 0xB400) {
        // THUMB.14 push/pop registers
        return &ARM7::Thumb_PushPop<(instruction >> 11) & 1,  /* 0=PUSH 1=POP */
                                (instruction >>  8) & 1>; /* PC/LR-flag */
    }
    if ((instruction & 0xF000) == 0xC000) {
        // THUMB.15 Multiple load/store
        return &ARM7::Thumb_LoadStoreMultiple<(instruction >> 11) & 1,  /* Load-Flag */
                                (instruction >>  8) & 7>; /* rB */
    }
    if ((instruction & 0xFF00) < 0xDF00) {
        // THUMB.16 Conditional Branch
        return &ARM7::Thumb_ConditionalBranch<(instruction >> 8) & 0xF>; /* Condition */
    }
    if ((instruction & 0xFF00) == 0xDF00) {
        // THUMB.17 Software Interrupt
        return &ARM7::Thumb_SWI;
    }
    if ((instruction & 0xF800) == 0xE000) {
        // THUMB.18 Unconditional Branch
        return &ARM7::Thumb_UnconditionalBranch;
    }
    if ((instruction & 0xF000) == 0xF000) {
        // THUMB.19 Long branch with link
        return &ARM7::Thumb_LongBranchLink<(instruction >> 11) & 1>; /* Second Instruction */
    }

    return &ARM7::Thumb_Undefined;
}

constexpr std::array<ARM7::ThumbInstruction, 1024> ARM7::MakeThumbLut() {
    std::array<ARM7::ThumbInstruction, 1024> lut = {};
    
    static_for<std::size_t, 0, 1024>([&](auto i) {
        lut[i] = GetThumbHandler<i<<6>();
    });
    return lut;
}
