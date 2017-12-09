/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
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

#include "arm.hpp"
#include "util/logger.hpp"

using namespace Util;

#define PREFETCH_T(accessType) \
    ctx.pipe[0] = ctx.pipe[1];\
    ctx.pipe[1] = busRead16(ctx.r15, accessType);

#define ADVANCE_PC ctx.r15 += 2;

#define REFILL_PIPELINE_A \
    ctx.pipe[0] = busRead32(ctx.r15,     M_NONSEQ);\
    ctx.pipe[1] = busRead32(ctx.r15 + 4, M_SEQ);\
    ctx.r15 += 8;

#define REFILL_PIPELINE_T \
    ctx.pipe[0] = busRead16(ctx.r15,     M_NONSEQ);\
    ctx.pipe[1] = busRead16(ctx.r15 + 2, M_SEQ);\
    ctx.r15 += 4;

namespace Core {

    template <int imm, int type>
    void ARM::thumbInst1(u16 instruction) {
        // THUMB.1 Move shifted register
        int  dst   = (instruction >> 0) & 7;
        int  src   = (instruction >> 3) & 7;
        bool carry = ctx.cpsr & MASK_CFLAG;

        PREFETCH_T(M_SEQ);

        ctx.reg[dst] = ctx.reg[src];

        applyShift(type, ctx.reg[dst], imm, carry, true);

        // update carry, sign and zero flag
        updateCarryFlag(carry);
        updateSignFlag(ctx.reg[dst]);
        updateZeroFlag(ctx.reg[dst]);

        ADVANCE_PC;
    }

    template <bool immediate, bool subtract, int field3>
    void ARM::thumbInst2(u16 instruction) {
        // THUMB.2 Add/subtract
        u32 operand;
        int dst = (instruction >> 0) & 7;
        int src = (instruction >> 3) & 7;

        PREFETCH_T(M_SEQ);

        // Third parameter. 3-bit immediate value (#imm) or register (rOP).
        operand = immediate ? field3 : ctx.reg[field3];

        if (subtract) {
            // SUB rDST, rSRC, operand
            ctx.reg[dst] = opSUB(ctx.reg[src], operand, true);
        } else {
            // ADD rDST, rSRC, operand
            ctx.reg[dst] = opADD(ctx.reg[src], operand, true);
        }

        ADVANCE_PC;
    }

    template <int op, int dst>
    void ARM::thumbInst3(u16 instruction) {
        // THUMB.3 Move/compare/add/subtract immediate
        u32 imm = instruction & 0xFF;

        PREFETCH_T(M_SEQ);

        switch (op) {
        case 0b00:
            // MOV rDST, #imm
            ctx.reg[dst] = imm;
            updateSignFlag(0);
            updateZeroFlag(imm);
            break;
        case 0b01:
            // CMP rDST, #imm
            opSUB(ctx.reg[dst], imm, true);
            break;
        case 0b10:
            // ADD rDST, #imm
            ctx.reg[dst] = opADD(ctx.reg[dst], imm, true);
            break;
        case 0b11:
            // SUB rDST, #imm
            ctx.reg[dst] = opSUB(ctx.reg[dst], imm, true);
            break;
        }

        ADVANCE_PC;
    }

    enum class ThumbDataOp {
        AND = 0,
        EOR = 1,
        LSL = 2,
        LSR = 3,
        ASR = 4,
        ADC = 5,
        SBC = 6,
        ROR = 7,
        TST = 8,
        NEG = 9,
        CMP = 10,
        CMN = 11,
        ORR = 12,
        MUL = 13,
        BIC = 14,
        MVN = 15
    };

    template <int op>
    void ARM::thumbInst4(u16 instruction) {
        // THUMB.4 ALU operations
        int dst = (instruction >> 0) & 7;
        int src = (instruction >> 3) & 7;

        PREFETCH_T(M_SEQ);

        switch (static_cast<ThumbDataOp>(op)) {
        case ThumbDataOp::AND: ctx.reg[dst] = opDataProc(ctx.reg[dst] & ctx.reg[src], true); break;
        case ThumbDataOp::EOR: ctx.reg[dst] = opDataProc(ctx.reg[dst] ^ ctx.reg[src], true); break;

        case ThumbDataOp::LSL: {
            u32 amount = ctx.reg[src];
            bool carry = ctx.cpsr & MASK_CFLAG;
            shiftLSL(ctx.reg[dst], amount, carry);
            updateCarryFlag(carry);
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            break;
        }

        case ThumbDataOp::LSR: {
            u32 amount = ctx.reg[src];
            bool carry = ctx.cpsr & MASK_CFLAG;
            shiftLSR(ctx.reg[dst], amount, carry, false);
            updateCarryFlag(carry);
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            break;
        }
        case ThumbDataOp::ASR: {
            u32 amount = ctx.reg[src];
            bool carry = ctx.cpsr & MASK_CFLAG;
            shiftASR(ctx.reg[dst], amount, carry, false);
            updateCarryFlag(carry);
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            break;
        }

        case ThumbDataOp::ADC: ctx.reg[dst] = opADC(ctx.reg[dst], ctx.reg[src], ( (ctx.cpsr)>>POS_CFLAG)&1, true); break;
        case ThumbDataOp::SBC: ctx.reg[dst] = opSBC(ctx.reg[dst], ctx.reg[src], (~(ctx.cpsr)>>POS_CFLAG)&1, true); break;

        case ThumbDataOp::ROR: {
            u32 amount = ctx.reg[src];
            bool carry = ctx.cpsr & MASK_CFLAG;
            shiftROR(ctx.reg[dst], amount, carry, false);
            updateCarryFlag(carry);
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            break;
        }

        case ThumbDataOp::TST:                opDataProc(ctx.reg[dst] & ctx.reg[src], true); break;
        case ThumbDataOp::NEG: ctx.reg[dst] = opSUB(0,            ctx.reg[src], true); break;
        case ThumbDataOp::CMP:                opSUB(ctx.reg[dst], ctx.reg[src], true); break;
        case ThumbDataOp::CMN:                opADD(ctx.reg[dst], ctx.reg[src], true); break;
        case ThumbDataOp::ORR: ctx.reg[dst] = opDataProc(ctx.reg[dst] | ctx.reg[src], true); break;

        case ThumbDataOp::MUL: {
            // TODO: calculate internal cycles
            ctx.reg[dst] *= ctx.reg[src];

            // Calculate flags. Is the carry flag really cleared?
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            updateCarryFlag(false);
            break;
        }

        case ThumbDataOp::BIC: ctx.reg[dst] = opDataProc(ctx.reg[dst] & ~ctx.reg[src], true); break;
        case ThumbDataOp::MVN: ctx.reg[dst] = opDataProc(~(ctx.reg[src]), true); break;
        }

        ADVANCE_PC;
    }

    template <int op, bool high1, bool high2>
    void ARM::thumbInst5(u16 instruction) {
        // THUMB.5 Hi register operations/branch exchange
        int dst = (instruction >> 0) & 7;
        int src = (instruction >> 3) & 7;

        // Instruction may access higher registers r8 - r15 ("Hi register").
        // This is archieved using two extra bits that displace the register number by 8.
        if (high1) dst |= 8;
        if (high2) src |= 8;

        PREFETCH_T(M_SEQ);

        u32 operand = ctx.reg[src];

        // If r15 is used as operand we must
        // ensure that its value is halfword aligned.
        if (src == 15) operand &= ~1;

        // Check for Branch & Exchange (bx) instruction.
        if (op == 3) {
            // LSB indicates new instruction set (0 = ARM, 1 = THUMB)
            if (operand & 1) {
                ctx.r15 = operand & ~1;
                REFILL_PIPELINE_T;
            } else {
                ctx.cpsr &= ~MASK_THUMB;
                ctx.r15   = operand & ~3;
                REFILL_PIPELINE_A;
            }
        }
        // Check for Compare (cmp) instruction
        else if (op == 1) {
            // This is a simple one. Subtraction but only the flags are stored.
            opSUB(ctx.reg[dst], operand, true);
            ADVANCE_PC;
        }
        // Otherwise it's an 'add' or 'mov' opcode
        else {
            switch (op) {
                case 0: ctx.reg[dst] += operand; break; // ADD
                case 2: ctx.reg[dst]  = operand; break; // MOV
                
                default: 
                    Logger::log<LOG_ERROR>("THUMB5: invalid opcode.");
                    break;
            }

            // Check if r15 was overwritten.
            // If so we need to realign it and refill the pipeline.
            if (dst == 15) {
                ctx.r15 &= ~1;
                REFILL_PIPELINE_T;
                return;
            }
            ADVANCE_PC;
        }
    }

    template <int dst>
    void ARM::thumbInst6(u16 instruction) {
        // THUMB.6 PC-relative load
        u32 imm     = instruction & 0xFF;
        u32 address = (ctx.r15 & ~2) + (imm << 2);

        PREFETCH_T(M_NONSEQ);
        busInternalCycles(1);
        ctx.reg[dst] = read32(address, M_NONSEQ);
        ADVANCE_PC;
    }

    template <int op, int off>
    void ARM::thumbInst7(u16 instruction) {
        // THUMB.7 Load/store with register offset
        int dst = instruction & 7;
        int base = (instruction >> 3) & 7;
        u32 address = ctx.reg[base] + ctx.reg[off];

        PREFETCH_T(M_NONSEQ);

        switch (op) {
        case 0b00: // STR
            write32(address, ctx.reg[dst], M_NONSEQ);
            break;
        case 0b01: // STRB
            write8(address, (u8)ctx.reg[dst], M_NONSEQ);
            break;
        case 0b10: // LDR
            busInternalCycles(1);
            ctx.reg[dst] = read32(address, M_NONSEQ | M_ROTATE);
            break;
        case 0b11: // LDRB
            busInternalCycles(1);
            ctx.reg[dst] = read8(address, M_NONSEQ);
            break;
        }

        ADVANCE_PC;
    }

    template <int op, int off>
    void ARM::thumbInst8(u16 instruction) {
        // THUMB.8 Load/store sign-extended byte/halfword
        int dst  = (instruction >> 0) & 7;
        int base = (instruction >> 3) & 7;
        u32 address = ctx.reg[base] + ctx.reg[off];

        PREFETCH_T(M_NONSEQ);

        switch (op) {
        case 0b00:
            // STRH rDST, [rBASE, rOFF]
            write16(address, ctx.reg[dst], M_NONSEQ);
            break;
        case 0b01:
            // LDSB rDST, [rBASE, rOFF]
            busInternalCycles(1);
            ctx.reg[dst] = read8(address, M_NONSEQ | M_SIGNED);
            break;
        case 0b10:
            // LDRH rDST, [rBASE, rOFF]
            busInternalCycles(1);
            ctx.reg[dst] = read16(address, M_NONSEQ | M_ROTATE);
            break;
        case 0b11:
            // LDSH rDST, [rBASE, rOFF]
            busInternalCycles(1);
            ctx.reg[dst] = read16(address, M_NONSEQ | M_SIGNED);
            break;
        }

        ADVANCE_PC;
    }

    template <int op, int imm>
    void ARM::thumbInst9(u16 instruction) {
        // THUMB.9 Load store with immediate offset
        int dst  = (instruction >> 0) & 7;
        int base = (instruction >> 3) & 7;

        PREFETCH_T(M_NONSEQ);

        switch (op) {
        case 0b00:
            // STR rDST, [rBASE, #imm]
            write32(
                ctx.reg[base] + (imm << 2),
                ctx.reg[dst],
                M_NONSEQ
            );
            break;
        case 0b01:
            // LDR rDST, [rBASE, #imm]
            // Internal CPU cycle (I-cycle)
            busInternalCycles(1);

            ctx.reg[dst] = read32(
                ctx.reg[base] + (imm << 2),
                M_NONSEQ | M_ROTATE
            );
            break;
        case 0b10:
            // STRB rDST, [rBASE, #imm]
            write8(
                ctx.reg[base] + imm,
                ctx.reg[dst],
                M_NONSEQ
            );
            break;
        case 0b11:
            // LDRB rDST, [rBASE, #imm]
            // Internal CPU cycle (I-cycle)
            busInternalCycles(1);

            ctx.reg[dst] = read8(
                ctx.reg[base] + imm,
                M_NONSEQ
            );
            break;
        }

        ADVANCE_PC;
    }

    template <bool load, int imm>
    void ARM::thumbInst10(u16 instruction) {
        // THUMB.10 Load/store halfword
        int dst     = (instruction >> 0) & 7;
        int base    = (instruction >> 3) & 7;
        u32 address = ctx.reg[base] + (imm << 1);

        PREFETCH_T(M_NONSEQ);

        if (load) {
            busInternalCycles(1);
            ctx.reg[dst] = read16(address, M_NONSEQ | M_ROTATE);
        } else {
            write16(address, ctx.reg[dst], M_NONSEQ);
        }

        ADVANCE_PC;
    }

    // touched
    template <bool load, int dst>
    void ARM::thumbInst11(u16 instruction) {
        // THUMB.11 SP-relative load/store
        u32 imm     = instruction & 0xFF;
        u32 address = ctx.reg[13] + (imm << 2);

        if (load) {
            busInternalCycles(1);
            ctx.reg[dst] = read32(address, M_NONSEQ | M_ROTATE);
            PREFETCH_T(M_NONSEQ);
        } else {
            write32(address, ctx.reg[dst], M_NONSEQ);
            PREFETCH_T(M_NONSEQ);
        }

        ADVANCE_PC;
    }

    template <bool stackptr, int dst>
    void ARM::thumbInst12(u16 instruction) {
        // THUMB.12 Load address
        u32 imm = (instruction & 0xFF) << 2;

        PREFETCH_T(M_SEQ);

        ctx.reg[dst] = (stackptr ? ctx.reg[13] : (ctx.r15 & ~2)) + imm;

        ADVANCE_PC;
    }

    template <bool sub>
    void ARM::thumbInst13(u16 instruction) {
        // THUMB.13 Add offset to stack pointer
        u32 imm = (instruction & 0x7F) << 2;

        PREFETCH_T(M_SEQ);

        ctx.reg[13] += sub ? -imm : imm;

        ADVANCE_PC;
    }

    template <bool pop, bool rbit>
    void ARM::thumbInst14(u16 instruction) {
        // THUMB.14 push/pop registers
        u32 addr = ctx.reg[13];

        PREFETCH_T(M_SEQ);

        // TODO: - emulate empty register list
        //       - figure proper timings & access orders.

        if (pop) {
            for (int reg = 0; reg <= 7; reg++) {
                if (instruction & (1<<reg)) {
                    ctx.reg[reg] = read32(addr, M_NONE);
                    addr += 4;
                }
            }
            if (rbit) {
                ctx.reg[15] = read32(addr, M_NONE) & ~1;
                ctx.reg[13] = addr + 4;
                REFILL_PIPELINE_T;
                return;
            }
            ctx.reg[13] = addr;
        } else {
            // Calculate internal start address (final r13 value)
            for (int reg = 0; reg <= 7; reg++) {
                if (instruction & (1 << reg)) addr -= 4;
            }
            if (rbit) addr -= 4;

            // Store address in r13 before we mess with it.
            ctx.reg[13] = addr;

            for (int reg = 0; reg <= 7; reg++) {
                if (instruction & (1<<reg)) {
                    write32(addr, ctx.reg[reg], M_NONE);
                    addr += 4;
                }
            }
            if (rbit) {
                write32(addr, ctx.reg[14], M_NONE);
            }
        }

        ADVANCE_PC;
    }

    // TODO: could use some optimizations :S
    template <bool load, int base>
    void ARM::thumbInst15(u16 instruction) {
        // THUMB.15 Multiple load/store
#if 0
        // TODO: verify on hardware
        if ((instruction & 0xFF) == 0) {
            PREFETCH_T(M_SEQ);
            ctx.reg[base] += 0x40;
            if (load) {
                ctx.r15 = read32(address, M_NONE); // check access type
                REFILL_PIPELINE_T;
            } else {
                write32(address, ctx.r15, M_NONE); // check access type
                ADVANCE_PC;
            }
            return;
        }
#endif

        if (load) {
            u32 address = ctx.reg[base];

            PREFETCH_T(M_SEQ);
            for (int i = 0; i <= 7; i++) {
                if (instruction & (1<<i)) {
                    ctx.reg[i] = read32(address, M_NONE);
                    address += 4;
                }
            }
            if (~instruction & (1<<base)) {
                ctx.reg[base] = address;
            }
        } else {
            PREFETCH_T(M_NONSEQ);

            int reg = 0;

            // First Loop - Run to first register (nonsequential access)
            for (; reg <= 7; reg++) {
                if (instruction & (1 << reg)) {
                    write32(ctx.reg[base], ctx.reg[reg], M_NONSEQ);
                    ctx.reg[base] += 4;
                    break;
                }
            }
            reg++;
            // Second Loop - Run until end (sequential accesses)
            for (; reg <= 7; reg++) {
                if (instruction & (1 << reg)) {
                    write32(ctx.reg[base], ctx.reg[reg], M_SEQ);
                    ctx.reg[base] += 4;
                }
            }

            /*for (int i = 0; i <= 7; i++) {
                if (instruction & (1<<i)) {
                    int access_type = M_SEQ; // ewww

                    if (first_register == -1) {
                        first_register = i;
                        access_type = M_NONSEQ;
                    }

                    //if (i == base && i == first_register) {
                    //    write32(ctx.reg[base], address, access_type);
                    //} else {
                    //    write32(ctx.reg[base], ctx.reg[i], access_type);
                    //}
                    write32(ctx.reg[base], ctx.reg[i], access_type);

                    ctx.reg[base] += 4;
                }
            }*/
        }

        ADVANCE_PC;
    }

    template <int cond>
    void ARM::thumbInst16(u16 instruction) {
        // THUMB.16 Conditional branch
        PREFETCH_T(M_SEQ);

        if (checkCondition(static_cast<Condition>(cond))) {
            u32 signed_immediate = instruction & 0xFF;

            // sign-extend the immediate value if neccessary
            if (signed_immediate & 0x80) {
                signed_immediate |= 0xFFFFFF00;
            }

            // update r15/pc and flush pipe
            ctx.r15 += (signed_immediate << 1);
            REFILL_PIPELINE_T;
        } else {
            ADVANCE_PC;
        }
    }

    void ARM::thumbInst17(u16 instruction) {
        // THUMB.17 Software Interrupt
        u8 call_number = read8(ctx.r15 - 4, M_NONE);

        PREFETCH_T(M_SEQ);

        if (!fake_swi) {
            Logger::log<LOG_DEBUG>("SWI[0x{0:X}]: r0=0x{1:X}, r1=0x{2:X}, r2=0x{3:X}", call_number, ctx.r0, ctx.r1, ctx.r2);

            // save return address and program status
            ctx.bank[BANK_SVC][BANK_R14] = ctx.r15 - 2;
            ctx.spsr[SPSR_SVC] = ctx.cpsr;

            // switch to SVC mode and disable interrupts
            switchMode(MODE_SVC);
            ctx.cpsr = (ctx.cpsr & ~MASK_THUMB) | MASK_IRQD;

            // jump to exception vector
            ctx.r15 = EXCPT_SWI;
            REFILL_PIPELINE_A;
        } else {
            handleSWI(call_number);
            ADVANCE_PC;
        }
    }

    void ARM::thumbInst18(u16 instruction) {
        // THUMB.18 Unconditional branch
        u32 imm = (instruction & 0x3FF) << 1;

        PREFETCH_T(M_SEQ);

        // sign-extend r15/pc displacement
        if (instruction & 0x400) {
            imm |= 0xFFFFF800;
        }

        // update r15/pc and flush pipe
        ctx.r15 += imm;
        REFILL_PIPELINE_T;
    }

    template <bool second_instruction>
    void ARM::thumbInst19(u16 instruction) {
        // THUMB.19 Long branch with link.
        u32 imm = instruction & 0x7FF;

        PREFETCH_T(M_SEQ);

        if (!second_instruction) {
            imm <<= 12;
            if (imm & 0x400000) {
                imm |= 0xFF800000;
            }
            ctx.r14 = ctx.r15 + imm;
            ADVANCE_PC;
        } else {
            u32 temp_pc = ctx.r15 - 2;

            // update r15/pc
            ctx.r15 = (ctx.r14 + (imm << 1)) & ~1;

            // store return address and flush pipe.
            ctx.reg[14] = temp_pc | 1;
            REFILL_PIPELINE_T;
        }
    }

    // - Instruction Handler LUT -
    // Maps an instruction opcode to a precompiled handler method.
    const ARM::ThumbInstruction ARM::thumb_lut[1024] = {
        /* THUMB.1 Move shifted register */
        &ARM::thumbInst1<0,0>,  &ARM::thumbInst1<1,0>,  &ARM::thumbInst1<2,0>,  &ARM::thumbInst1<3,0>,
        &ARM::thumbInst1<4,0>,  &ARM::thumbInst1<5,0>,  &ARM::thumbInst1<6,0>,  &ARM::thumbInst1<7,0>,
        &ARM::thumbInst1<8,0>,  &ARM::thumbInst1<9,0>,  &ARM::thumbInst1<10,0>, &ARM::thumbInst1<11,0>,
        &ARM::thumbInst1<12,0>, &ARM::thumbInst1<13,0>, &ARM::thumbInst1<14,0>, &ARM::thumbInst1<15,0>,
        &ARM::thumbInst1<16,0>, &ARM::thumbInst1<17,0>, &ARM::thumbInst1<18,0>, &ARM::thumbInst1<19,0>,
        &ARM::thumbInst1<20,0>, &ARM::thumbInst1<21,0>, &ARM::thumbInst1<22,0>, &ARM::thumbInst1<23,0>,
        &ARM::thumbInst1<24,0>, &ARM::thumbInst1<25,0>, &ARM::thumbInst1<26,0>, &ARM::thumbInst1<27,0>,
        &ARM::thumbInst1<28,0>, &ARM::thumbInst1<29,0>, &ARM::thumbInst1<30,0>, &ARM::thumbInst1<31,0>,
        &ARM::thumbInst1<0,1>,  &ARM::thumbInst1<1,1>,  &ARM::thumbInst1<2,1>,  &ARM::thumbInst1<3,1>,
        &ARM::thumbInst1<4,1>,  &ARM::thumbInst1<5,1>,  &ARM::thumbInst1<6,1>,  &ARM::thumbInst1<7,1>,
        &ARM::thumbInst1<8,1>,  &ARM::thumbInst1<9,1>,  &ARM::thumbInst1<10,1>, &ARM::thumbInst1<11,1>,
        &ARM::thumbInst1<12,1>, &ARM::thumbInst1<13,1>, &ARM::thumbInst1<14,1>, &ARM::thumbInst1<15,1>,
        &ARM::thumbInst1<16,1>, &ARM::thumbInst1<17,1>, &ARM::thumbInst1<18,1>, &ARM::thumbInst1<19,1>,
        &ARM::thumbInst1<20,1>, &ARM::thumbInst1<21,1>, &ARM::thumbInst1<22,1>, &ARM::thumbInst1<23,1>,
        &ARM::thumbInst1<24,1>, &ARM::thumbInst1<25,1>, &ARM::thumbInst1<26,1>, &ARM::thumbInst1<27,1>,
        &ARM::thumbInst1<28,1>, &ARM::thumbInst1<29,1>, &ARM::thumbInst1<30,1>, &ARM::thumbInst1<31,1>,
        &ARM::thumbInst1<0,2>,  &ARM::thumbInst1<1,2>,  &ARM::thumbInst1<2,2>,  &ARM::thumbInst1<3,2>,
        &ARM::thumbInst1<4,2>,  &ARM::thumbInst1<5,2>,  &ARM::thumbInst1<6,2>,  &ARM::thumbInst1<7,2>,
        &ARM::thumbInst1<8,2>,  &ARM::thumbInst1<9,2>,  &ARM::thumbInst1<10,2>, &ARM::thumbInst1<11,2>,
        &ARM::thumbInst1<12,2>, &ARM::thumbInst1<13,2>, &ARM::thumbInst1<14,2>, &ARM::thumbInst1<15,2>,
        &ARM::thumbInst1<16,2>, &ARM::thumbInst1<17,2>, &ARM::thumbInst1<18,2>, &ARM::thumbInst1<19,2>,
        &ARM::thumbInst1<20,2>, &ARM::thumbInst1<21,2>, &ARM::thumbInst1<22,2>, &ARM::thumbInst1<23,2>,
        &ARM::thumbInst1<24,2>, &ARM::thumbInst1<25,2>, &ARM::thumbInst1<26,2>, &ARM::thumbInst1<27,2>,
        &ARM::thumbInst1<28,2>, &ARM::thumbInst1<29,2>, &ARM::thumbInst1<30,2>, &ARM::thumbInst1<31,2>,

        /* THUMB.2 Add / subtract */
        &ARM::thumbInst2<0,0,0>, &ARM::thumbInst2<0,0,1>, &ARM::thumbInst2<0,0,2>, &ARM::thumbInst2<0,0,3>,
        &ARM::thumbInst2<0,0,4>, &ARM::thumbInst2<0,0,5>, &ARM::thumbInst2<0,0,6>, &ARM::thumbInst2<0,0,7>,
        &ARM::thumbInst2<0,1,0>, &ARM::thumbInst2<0,1,1>, &ARM::thumbInst2<0,1,2>, &ARM::thumbInst2<0,1,3>,
        &ARM::thumbInst2<0,1,4>, &ARM::thumbInst2<0,1,5>, &ARM::thumbInst2<0,1,6>, &ARM::thumbInst2<0,1,7>,
        &ARM::thumbInst2<1,0,0>, &ARM::thumbInst2<1,0,1>, &ARM::thumbInst2<1,0,2>, &ARM::thumbInst2<1,0,3>,
        &ARM::thumbInst2<1,0,4>, &ARM::thumbInst2<1,0,5>, &ARM::thumbInst2<1,0,6>, &ARM::thumbInst2<1,0,7>,
        &ARM::thumbInst2<1,1,0>, &ARM::thumbInst2<1,1,1>, &ARM::thumbInst2<1,1,2>, &ARM::thumbInst2<1,1,3>,
        &ARM::thumbInst2<1,1,4>, &ARM::thumbInst2<1,1,5>, &ARM::thumbInst2<1,1,6>, &ARM::thumbInst2<1,1,7>,

        /* THUMB.3 Move/compare/add/subtract immediate */
        &ARM::thumbInst3<0,0>, &ARM::thumbInst3<0,0>, &ARM::thumbInst3<0,0>, &ARM::thumbInst3<0,0>,
        &ARM::thumbInst3<0,1>, &ARM::thumbInst3<0,1>, &ARM::thumbInst3<0,1>, &ARM::thumbInst3<0,1>,
        &ARM::thumbInst3<0,2>, &ARM::thumbInst3<0,2>, &ARM::thumbInst3<0,2>, &ARM::thumbInst3<0,2>,
        &ARM::thumbInst3<0,3>, &ARM::thumbInst3<0,3>, &ARM::thumbInst3<0,3>, &ARM::thumbInst3<0,3>,
        &ARM::thumbInst3<0,4>, &ARM::thumbInst3<0,4>, &ARM::thumbInst3<0,4>, &ARM::thumbInst3<0,4>,
        &ARM::thumbInst3<0,5>, &ARM::thumbInst3<0,5>, &ARM::thumbInst3<0,5>, &ARM::thumbInst3<0,5>,
        &ARM::thumbInst3<0,6>, &ARM::thumbInst3<0,6>, &ARM::thumbInst3<0,6>, &ARM::thumbInst3<0,6>,
        &ARM::thumbInst3<0,7>, &ARM::thumbInst3<0,7>, &ARM::thumbInst3<0,7>, &ARM::thumbInst3<0,7>,
        &ARM::thumbInst3<1,0>, &ARM::thumbInst3<1,0>, &ARM::thumbInst3<1,0>, &ARM::thumbInst3<1,0>,
        &ARM::thumbInst3<1,1>, &ARM::thumbInst3<1,1>, &ARM::thumbInst3<1,1>, &ARM::thumbInst3<1,1>,
        &ARM::thumbInst3<1,2>, &ARM::thumbInst3<1,2>, &ARM::thumbInst3<1,2>, &ARM::thumbInst3<1,2>,
        &ARM::thumbInst3<1,3>, &ARM::thumbInst3<1,3>, &ARM::thumbInst3<1,3>, &ARM::thumbInst3<1,3>,
        &ARM::thumbInst3<1,4>, &ARM::thumbInst3<1,4>, &ARM::thumbInst3<1,4>, &ARM::thumbInst3<1,4>,
        &ARM::thumbInst3<1,5>, &ARM::thumbInst3<1,5>, &ARM::thumbInst3<1,5>, &ARM::thumbInst3<1,5>,
        &ARM::thumbInst3<1,6>, &ARM::thumbInst3<1,6>, &ARM::thumbInst3<1,6>, &ARM::thumbInst3<1,6>,
        &ARM::thumbInst3<1,7>, &ARM::thumbInst3<1,7>, &ARM::thumbInst3<1,7>, &ARM::thumbInst3<1,7>,
        &ARM::thumbInst3<2,0>, &ARM::thumbInst3<2,0>, &ARM::thumbInst3<2,0>, &ARM::thumbInst3<2,0>,
        &ARM::thumbInst3<2,1>, &ARM::thumbInst3<2,1>, &ARM::thumbInst3<2,1>, &ARM::thumbInst3<2,1>,
        &ARM::thumbInst3<2,2>, &ARM::thumbInst3<2,2>, &ARM::thumbInst3<2,2>, &ARM::thumbInst3<2,2>,
        &ARM::thumbInst3<2,3>, &ARM::thumbInst3<2,3>, &ARM::thumbInst3<2,3>, &ARM::thumbInst3<2,3>,
        &ARM::thumbInst3<2,4>, &ARM::thumbInst3<2,4>, &ARM::thumbInst3<2,4>, &ARM::thumbInst3<2,4>,
        &ARM::thumbInst3<2,5>, &ARM::thumbInst3<2,5>, &ARM::thumbInst3<2,5>, &ARM::thumbInst3<2,5>,
        &ARM::thumbInst3<2,6>, &ARM::thumbInst3<2,6>, &ARM::thumbInst3<2,6>, &ARM::thumbInst3<2,6>,
        &ARM::thumbInst3<2,7>, &ARM::thumbInst3<2,7>, &ARM::thumbInst3<2,7>, &ARM::thumbInst3<2,7>,
        &ARM::thumbInst3<3,0>, &ARM::thumbInst3<3,0>, &ARM::thumbInst3<3,0>, &ARM::thumbInst3<3,0>,
        &ARM::thumbInst3<3,1>, &ARM::thumbInst3<3,1>, &ARM::thumbInst3<3,1>, &ARM::thumbInst3<3,1>,
        &ARM::thumbInst3<3,2>, &ARM::thumbInst3<3,2>, &ARM::thumbInst3<3,2>, &ARM::thumbInst3<3,2>,
        &ARM::thumbInst3<3,3>, &ARM::thumbInst3<3,3>, &ARM::thumbInst3<3,3>, &ARM::thumbInst3<3,3>,
        &ARM::thumbInst3<3,4>, &ARM::thumbInst3<3,4>, &ARM::thumbInst3<3,4>, &ARM::thumbInst3<3,4>,
        &ARM::thumbInst3<3,5>, &ARM::thumbInst3<3,5>, &ARM::thumbInst3<3,5>, &ARM::thumbInst3<3,5>,
        &ARM::thumbInst3<3,6>, &ARM::thumbInst3<3,6>, &ARM::thumbInst3<3,6>, &ARM::thumbInst3<3,6>,
        &ARM::thumbInst3<3,7>, &ARM::thumbInst3<3,7>, &ARM::thumbInst3<3,7>, &ARM::thumbInst3<3,7>,

        /* THUMB.4 ALU operations */
        &ARM::thumbInst4<0>,  &ARM::thumbInst4<1>,  &ARM::thumbInst4<2>,  &ARM::thumbInst4<3>,
        &ARM::thumbInst4<4>,  &ARM::thumbInst4<5>,  &ARM::thumbInst4<6>,  &ARM::thumbInst4<7>,
        &ARM::thumbInst4<8>,  &ARM::thumbInst4<9>,  &ARM::thumbInst4<10>, &ARM::thumbInst4<11>,
        &ARM::thumbInst4<12>, &ARM::thumbInst4<13>, &ARM::thumbInst4<14>, &ARM::thumbInst4<15>,

        /* THUMB.5 High register operations/branch exchange
         * TODO: Eventually move BX into it's own method. */
        &ARM::thumbInst5<0,0,0>, &ARM::thumbInst5<0,0,1>, &ARM::thumbInst5<0,1,0>, &ARM::thumbInst5<0,1,1>,
        &ARM::thumbInst5<1,0,0>, &ARM::thumbInst5<1,0,1>, &ARM::thumbInst5<1,1,0>, &ARM::thumbInst5<1,1,1>,
        &ARM::thumbInst5<2,0,0>, &ARM::thumbInst5<2,0,1>, &ARM::thumbInst5<2,1,0>, &ARM::thumbInst5<2,1,1>,
        &ARM::thumbInst5<3,0,0>, &ARM::thumbInst5<3,0,1>, &ARM::thumbInst5<3,1,0>, &ARM::thumbInst5<3,1,1>,

        /* THUMB.6 PC-relative load */
        &ARM::thumbInst6<0>, &ARM::thumbInst6<0>, &ARM::thumbInst6<0>, &ARM::thumbInst6<0>,
        &ARM::thumbInst6<1>, &ARM::thumbInst6<1>, &ARM::thumbInst6<1>, &ARM::thumbInst6<1>,
        &ARM::thumbInst6<2>, &ARM::thumbInst6<2>, &ARM::thumbInst6<2>, &ARM::thumbInst6<2>,
        &ARM::thumbInst6<3>, &ARM::thumbInst6<3>, &ARM::thumbInst6<3>, &ARM::thumbInst6<3>,
        &ARM::thumbInst6<4>, &ARM::thumbInst6<4>, &ARM::thumbInst6<4>, &ARM::thumbInst6<4>,
        &ARM::thumbInst6<5>, &ARM::thumbInst6<5>, &ARM::thumbInst6<5>, &ARM::thumbInst6<5>,
        &ARM::thumbInst6<6>, &ARM::thumbInst6<6>, &ARM::thumbInst6<6>, &ARM::thumbInst6<6>,
        &ARM::thumbInst6<7>, &ARM::thumbInst6<7>, &ARM::thumbInst6<7>, &ARM::thumbInst6<7>,

        /* THUMB.7 Load/store with register offset,
           THUMB.8 Load/store sign-extended byte/halfword */
        &ARM::thumbInst7<0,0>, &ARM::thumbInst7<0,1>, &ARM::thumbInst7<0,2>, &ARM::thumbInst7<0,3>,
        &ARM::thumbInst7<0,4>, &ARM::thumbInst7<0,5>, &ARM::thumbInst7<0,6>, &ARM::thumbInst7<0,7>,
        &ARM::thumbInst8<0,0>, &ARM::thumbInst8<0,1>, &ARM::thumbInst8<0,2>, &ARM::thumbInst8<0,3>,
        &ARM::thumbInst8<0,4>, &ARM::thumbInst8<0,5>, &ARM::thumbInst8<0,6>, &ARM::thumbInst8<0,7>,
        &ARM::thumbInst7<1,0>, &ARM::thumbInst7<1,1>, &ARM::thumbInst7<1,2>, &ARM::thumbInst7<1,3>,
        &ARM::thumbInst7<1,4>, &ARM::thumbInst7<1,5>, &ARM::thumbInst7<1,6>, &ARM::thumbInst7<1,7>,
        &ARM::thumbInst8<1,0>, &ARM::thumbInst8<1,1>, &ARM::thumbInst8<1,2>, &ARM::thumbInst8<1,3>,
        &ARM::thumbInst8<1,4>, &ARM::thumbInst8<1,5>, &ARM::thumbInst8<1,6>, &ARM::thumbInst8<1,7>,
        &ARM::thumbInst7<2,0>, &ARM::thumbInst7<2,1>, &ARM::thumbInst7<2,2>, &ARM::thumbInst7<2,3>,
        &ARM::thumbInst7<2,4>, &ARM::thumbInst7<2,5>, &ARM::thumbInst7<2,6>, &ARM::thumbInst7<2,7>,
        &ARM::thumbInst8<2,0>, &ARM::thumbInst8<2,1>, &ARM::thumbInst8<2,2>, &ARM::thumbInst8<2,3>,
        &ARM::thumbInst8<2,4>, &ARM::thumbInst8<2,5>, &ARM::thumbInst8<2,6>, &ARM::thumbInst8<2,7>,
        &ARM::thumbInst7<3,0>, &ARM::thumbInst7<3,1>, &ARM::thumbInst7<3,2>, &ARM::thumbInst7<3,3>,
        &ARM::thumbInst7<3,4>, &ARM::thumbInst7<3,5>, &ARM::thumbInst7<3,6>, &ARM::thumbInst7<3,7>,
        &ARM::thumbInst8<3,0>, &ARM::thumbInst8<3,1>, &ARM::thumbInst8<3,2>, &ARM::thumbInst8<3,3>,
        &ARM::thumbInst8<3,4>, &ARM::thumbInst8<3,5>, &ARM::thumbInst8<3,6>, &ARM::thumbInst8<3,7>,

        /* THUMB.9 Load/store with immediate offset */
        &ARM::thumbInst9<0,0>,  &ARM::thumbInst9<0,1>,  &ARM::thumbInst9<0,2>,  &ARM::thumbInst9<0,3>,
        &ARM::thumbInst9<0,4>,  &ARM::thumbInst9<0,5>,  &ARM::thumbInst9<0,6>,  &ARM::thumbInst9<0,7>,
        &ARM::thumbInst9<0,8>,  &ARM::thumbInst9<0,9>,  &ARM::thumbInst9<0,10>, &ARM::thumbInst9<0,11>,
        &ARM::thumbInst9<0,12>, &ARM::thumbInst9<0,13>, &ARM::thumbInst9<0,14>, &ARM::thumbInst9<0,15>,
        &ARM::thumbInst9<0,16>, &ARM::thumbInst9<0,17>, &ARM::thumbInst9<0,18>, &ARM::thumbInst9<0,19>,
        &ARM::thumbInst9<0,20>, &ARM::thumbInst9<0,21>, &ARM::thumbInst9<0,22>, &ARM::thumbInst9<0,23>,
        &ARM::thumbInst9<0,24>, &ARM::thumbInst9<0,25>, &ARM::thumbInst9<0,26>, &ARM::thumbInst9<0,27>,
        &ARM::thumbInst9<0,28>, &ARM::thumbInst9<0,29>, &ARM::thumbInst9<0,30>, &ARM::thumbInst9<0,31>,
        &ARM::thumbInst9<1,0>,  &ARM::thumbInst9<1,1>,  &ARM::thumbInst9<1,2>,  &ARM::thumbInst9<1,3>,
        &ARM::thumbInst9<1,4>,  &ARM::thumbInst9<1,5>,  &ARM::thumbInst9<1,6>,  &ARM::thumbInst9<1,7>,
        &ARM::thumbInst9<1,8>,  &ARM::thumbInst9<1,9>,  &ARM::thumbInst9<1,10>, &ARM::thumbInst9<1,11>,
        &ARM::thumbInst9<1,12>, &ARM::thumbInst9<1,13>, &ARM::thumbInst9<1,14>, &ARM::thumbInst9<1,15>,
        &ARM::thumbInst9<1,16>, &ARM::thumbInst9<1,17>, &ARM::thumbInst9<1,18>, &ARM::thumbInst9<1,19>,
        &ARM::thumbInst9<1,20>, &ARM::thumbInst9<1,21>, &ARM::thumbInst9<1,22>, &ARM::thumbInst9<1,23>,
        &ARM::thumbInst9<1,24>, &ARM::thumbInst9<1,25>, &ARM::thumbInst9<1,26>, &ARM::thumbInst9<1,27>,
        &ARM::thumbInst9<1,28>, &ARM::thumbInst9<1,29>, &ARM::thumbInst9<1,30>, &ARM::thumbInst9<1,31>,
        &ARM::thumbInst9<2,0>,  &ARM::thumbInst9<2,1>,  &ARM::thumbInst9<2,2>,  &ARM::thumbInst9<2,3>,
        &ARM::thumbInst9<2,4>,  &ARM::thumbInst9<2,5>,  &ARM::thumbInst9<2,6>,  &ARM::thumbInst9<2,7>,
        &ARM::thumbInst9<2,8>,  &ARM::thumbInst9<2,9>,  &ARM::thumbInst9<2,10>, &ARM::thumbInst9<2,11>,
        &ARM::thumbInst9<2,12>, &ARM::thumbInst9<2,13>, &ARM::thumbInst9<2,14>, &ARM::thumbInst9<2,15>,
        &ARM::thumbInst9<2,16>, &ARM::thumbInst9<2,17>, &ARM::thumbInst9<2,18>, &ARM::thumbInst9<2,19>,
        &ARM::thumbInst9<2,20>, &ARM::thumbInst9<2,21>, &ARM::thumbInst9<2,22>, &ARM::thumbInst9<2,23>,
        &ARM::thumbInst9<2,24>, &ARM::thumbInst9<2,25>, &ARM::thumbInst9<2,26>, &ARM::thumbInst9<2,27>,
        &ARM::thumbInst9<2,28>, &ARM::thumbInst9<2,29>, &ARM::thumbInst9<2,30>, &ARM::thumbInst9<2,31>,
        &ARM::thumbInst9<3,0>,  &ARM::thumbInst9<3,1>,  &ARM::thumbInst9<3,2>,  &ARM::thumbInst9<3,3>,
        &ARM::thumbInst9<3,4>,  &ARM::thumbInst9<3,5>,  &ARM::thumbInst9<3,6>,  &ARM::thumbInst9<3,7>,
        &ARM::thumbInst9<3,8>,  &ARM::thumbInst9<3,9>,  &ARM::thumbInst9<3,10>, &ARM::thumbInst9<3,11>,
        &ARM::thumbInst9<3,12>, &ARM::thumbInst9<3,13>, &ARM::thumbInst9<3,14>, &ARM::thumbInst9<3,15>,
        &ARM::thumbInst9<3,16>, &ARM::thumbInst9<3,17>, &ARM::thumbInst9<3,18>, &ARM::thumbInst9<3,19>,
        &ARM::thumbInst9<3,20>, &ARM::thumbInst9<3,21>, &ARM::thumbInst9<3,22>, &ARM::thumbInst9<3,23>,
        &ARM::thumbInst9<3,24>, &ARM::thumbInst9<3,25>, &ARM::thumbInst9<3,26>, &ARM::thumbInst9<3,27>,
        &ARM::thumbInst9<3,28>, &ARM::thumbInst9<3,29>, &ARM::thumbInst9<3,30>, &ARM::thumbInst9<3,31>,

        /* THUMB.10 Load/store halfword */
        &ARM::thumbInst10<0,0>,  &ARM::thumbInst10<0,1>,  &ARM::thumbInst10<0,2>,  &ARM::thumbInst10<0,3>,
        &ARM::thumbInst10<0,4>,  &ARM::thumbInst10<0,5>,  &ARM::thumbInst10<0,6>,  &ARM::thumbInst10<0,7>,
        &ARM::thumbInst10<0,8>,  &ARM::thumbInst10<0,9>,  &ARM::thumbInst10<0,10>, &ARM::thumbInst10<0,11>,
        &ARM::thumbInst10<0,12>, &ARM::thumbInst10<0,13>, &ARM::thumbInst10<0,14>, &ARM::thumbInst10<0,15>,
        &ARM::thumbInst10<0,16>, &ARM::thumbInst10<0,17>, &ARM::thumbInst10<0,18>, &ARM::thumbInst10<0,19>,
        &ARM::thumbInst10<0,20>, &ARM::thumbInst10<0,21>, &ARM::thumbInst10<0,22>, &ARM::thumbInst10<0,23>,
        &ARM::thumbInst10<0,24>, &ARM::thumbInst10<0,25>, &ARM::thumbInst10<0,26>, &ARM::thumbInst10<0,27>,
        &ARM::thumbInst10<0,28>, &ARM::thumbInst10<0,29>, &ARM::thumbInst10<0,30>, &ARM::thumbInst10<0,31>,
        &ARM::thumbInst10<1,0>,  &ARM::thumbInst10<1,1>,  &ARM::thumbInst10<1,2>,  &ARM::thumbInst10<1,3>,
        &ARM::thumbInst10<1,4>,  &ARM::thumbInst10<1,5>,  &ARM::thumbInst10<1,6>,  &ARM::thumbInst10<1,7>,
        &ARM::thumbInst10<1,8>,  &ARM::thumbInst10<1,9>,  &ARM::thumbInst10<1,10>, &ARM::thumbInst10<1,11>,
        &ARM::thumbInst10<1,12>, &ARM::thumbInst10<1,13>, &ARM::thumbInst10<1,14>, &ARM::thumbInst10<1,15>,
        &ARM::thumbInst10<1,16>, &ARM::thumbInst10<1,17>, &ARM::thumbInst10<1,18>, &ARM::thumbInst10<1,19>,
        &ARM::thumbInst10<1,20>, &ARM::thumbInst10<1,21>, &ARM::thumbInst10<1,22>, &ARM::thumbInst10<1,23>,
        &ARM::thumbInst10<1,24>, &ARM::thumbInst10<1,25>, &ARM::thumbInst10<1,26>, &ARM::thumbInst10<1,27>,
        &ARM::thumbInst10<1,28>, &ARM::thumbInst10<1,29>, &ARM::thumbInst10<1,30>, &ARM::thumbInst10<1,31>,

        /* THUMB.11 SP-relative load/store */
        &ARM::thumbInst11<0,0>, &ARM::thumbInst11<0,0>, &ARM::thumbInst11<0,0>, &ARM::thumbInst11<0,0>,
        &ARM::thumbInst11<0,1>, &ARM::thumbInst11<0,1>, &ARM::thumbInst11<0,1>, &ARM::thumbInst11<0,1>,
        &ARM::thumbInst11<0,2>, &ARM::thumbInst11<0,2>, &ARM::thumbInst11<0,2>, &ARM::thumbInst11<0,2>,
        &ARM::thumbInst11<0,3>, &ARM::thumbInst11<0,3>, &ARM::thumbInst11<0,3>, &ARM::thumbInst11<0,3>,
        &ARM::thumbInst11<0,4>, &ARM::thumbInst11<0,4>, &ARM::thumbInst11<0,4>, &ARM::thumbInst11<0,4>,
        &ARM::thumbInst11<0,5>, &ARM::thumbInst11<0,5>, &ARM::thumbInst11<0,5>, &ARM::thumbInst11<0,5>,
        &ARM::thumbInst11<0,6>, &ARM::thumbInst11<0,6>, &ARM::thumbInst11<0,6>, &ARM::thumbInst11<0,6>,
        &ARM::thumbInst11<0,7>, &ARM::thumbInst11<0,7>, &ARM::thumbInst11<0,7>, &ARM::thumbInst11<0,7>,
        &ARM::thumbInst11<1,0>, &ARM::thumbInst11<1,0>, &ARM::thumbInst11<1,0>, &ARM::thumbInst11<1,0>,
        &ARM::thumbInst11<1,1>, &ARM::thumbInst11<1,1>, &ARM::thumbInst11<1,1>, &ARM::thumbInst11<1,1>,
        &ARM::thumbInst11<1,2>, &ARM::thumbInst11<1,2>, &ARM::thumbInst11<1,2>, &ARM::thumbInst11<1,2>,
        &ARM::thumbInst11<1,3>, &ARM::thumbInst11<1,3>, &ARM::thumbInst11<1,3>, &ARM::thumbInst11<1,3>,
        &ARM::thumbInst11<1,4>, &ARM::thumbInst11<1,4>, &ARM::thumbInst11<1,4>, &ARM::thumbInst11<1,4>,
        &ARM::thumbInst11<1,5>, &ARM::thumbInst11<1,5>, &ARM::thumbInst11<1,5>, &ARM::thumbInst11<1,5>,
        &ARM::thumbInst11<1,6>, &ARM::thumbInst11<1,6>, &ARM::thumbInst11<1,6>, &ARM::thumbInst11<1,6>,
        &ARM::thumbInst11<1,7>, &ARM::thumbInst11<1,7>, &ARM::thumbInst11<1,7>, &ARM::thumbInst11<1,7>,

        /* THUMB.12 Load address */
        &ARM::thumbInst12<0,0>, &ARM::thumbInst12<0,0>, &ARM::thumbInst12<0,0>, &ARM::thumbInst12<0,0>,
        &ARM::thumbInst12<0,1>, &ARM::thumbInst12<0,1>, &ARM::thumbInst12<0,1>, &ARM::thumbInst12<0,1>,
        &ARM::thumbInst12<0,2>, &ARM::thumbInst12<0,2>, &ARM::thumbInst12<0,2>, &ARM::thumbInst12<0,2>,
        &ARM::thumbInst12<0,3>, &ARM::thumbInst12<0,3>, &ARM::thumbInst12<0,3>, &ARM::thumbInst12<0,3>,
        &ARM::thumbInst12<0,4>, &ARM::thumbInst12<0,4>, &ARM::thumbInst12<0,4>, &ARM::thumbInst12<0,4>,
        &ARM::thumbInst12<0,5>, &ARM::thumbInst12<0,5>, &ARM::thumbInst12<0,5>, &ARM::thumbInst12<0,5>,
        &ARM::thumbInst12<0,6>, &ARM::thumbInst12<0,6>, &ARM::thumbInst12<0,6>, &ARM::thumbInst12<0,6>,
        &ARM::thumbInst12<0,7>, &ARM::thumbInst12<0,7>, &ARM::thumbInst12<0,7>, &ARM::thumbInst12<0,7>,
        &ARM::thumbInst12<1,0>, &ARM::thumbInst12<1,0>, &ARM::thumbInst12<1,0>, &ARM::thumbInst12<1,0>,
        &ARM::thumbInst12<1,1>, &ARM::thumbInst12<1,1>, &ARM::thumbInst12<1,1>, &ARM::thumbInst12<1,1>,
        &ARM::thumbInst12<1,2>, &ARM::thumbInst12<1,2>, &ARM::thumbInst12<1,2>, &ARM::thumbInst12<1,2>,
        &ARM::thumbInst12<1,3>, &ARM::thumbInst12<1,3>, &ARM::thumbInst12<1,3>, &ARM::thumbInst12<1,3>,
        &ARM::thumbInst12<1,4>, &ARM::thumbInst12<1,4>, &ARM::thumbInst12<1,4>, &ARM::thumbInst12<1,4>,
        &ARM::thumbInst12<1,5>, &ARM::thumbInst12<1,5>, &ARM::thumbInst12<1,5>, &ARM::thumbInst12<1,5>,
        &ARM::thumbInst12<1,6>, &ARM::thumbInst12<1,6>, &ARM::thumbInst12<1,6>, &ARM::thumbInst12<1,6>,
        &ARM::thumbInst12<1,7>, &ARM::thumbInst12<1,7>, &ARM::thumbInst12<1,7>, &ARM::thumbInst12<1,7>,

        /* THUMB.13 Add offset to stack pointer,
           THUMB.14 Push/pop registers */
        &ARM::thumbInst13<0>,   &ARM::thumbInst13<0>,   &ARM::thumbInst13<1>,   &ARM::thumbInst13<1>,
        &ARM::thumbInst13<0>,   &ARM::thumbInst13<0>,   &ARM::thumbInst13<1>,   &ARM::thumbInst13<1>,
        &ARM::thumbInst13<0>,   &ARM::thumbInst13<0>,   &ARM::thumbInst13<1>,   &ARM::thumbInst13<1>,
        &ARM::thumbInst13<0>,   &ARM::thumbInst13<0>,   &ARM::thumbInst13<1>,   &ARM::thumbInst13<1>,
        &ARM::thumbInst14<0,0>, &ARM::thumbInst14<0,0>, &ARM::thumbInst14<0,0>, &ARM::thumbInst14<0,0>,
        &ARM::thumbInst14<0,1>, &ARM::thumbInst14<0,1>, &ARM::thumbInst14<0,1>, &ARM::thumbInst14<0,1>,
        &ARM::thumbInst14<0,0>, &ARM::thumbInst14<0,0>, &ARM::thumbInst14<0,0>, &ARM::thumbInst14<0,0>,
        &ARM::thumbInst14<0,1>, &ARM::thumbInst14<0,1>, &ARM::thumbInst14<0,1>, &ARM::thumbInst14<0,1>,
        &ARM::thumbInst13<0>,   &ARM::thumbInst13<0>,   &ARM::thumbInst13<1>,   &ARM::thumbInst13<1>,
        &ARM::thumbInst13<0>,   &ARM::thumbInst13<0>,   &ARM::thumbInst13<1>,   &ARM::thumbInst13<1>,
        &ARM::thumbInst13<0>,   &ARM::thumbInst13<0>,   &ARM::thumbInst13<1>,   &ARM::thumbInst13<1>,
        &ARM::thumbInst13<0>,   &ARM::thumbInst13<0>,   &ARM::thumbInst13<1>,   &ARM::thumbInst13<1>,
        &ARM::thumbInst14<1,0>, &ARM::thumbInst14<1,0>, &ARM::thumbInst14<1,0>, &ARM::thumbInst14<1,0>,
        &ARM::thumbInst14<1,1>, &ARM::thumbInst14<1,1>, &ARM::thumbInst14<1,1>, &ARM::thumbInst14<1,1>,
        &ARM::thumbInst14<1,0>, &ARM::thumbInst14<1,0>, &ARM::thumbInst14<1,0>, &ARM::thumbInst14<1,0>,
        &ARM::thumbInst14<1,1>, &ARM::thumbInst14<1,1>, &ARM::thumbInst14<1,1>, &ARM::thumbInst14<1,1>,

        /* THUMB.15 Multiple load/store */
        &ARM::thumbInst15<0,0>, &ARM::thumbInst15<0,0>, &ARM::thumbInst15<0,0>, &ARM::thumbInst15<0,0>,
        &ARM::thumbInst15<0,1>, &ARM::thumbInst15<0,1>, &ARM::thumbInst15<0,1>, &ARM::thumbInst15<0,1>,
        &ARM::thumbInst15<0,2>, &ARM::thumbInst15<0,2>, &ARM::thumbInst15<0,2>, &ARM::thumbInst15<0,2>,
        &ARM::thumbInst15<0,3>, &ARM::thumbInst15<0,3>, &ARM::thumbInst15<0,3>, &ARM::thumbInst15<0,3>,
        &ARM::thumbInst15<0,4>, &ARM::thumbInst15<0,4>, &ARM::thumbInst15<0,4>, &ARM::thumbInst15<0,4>,
        &ARM::thumbInst15<0,5>, &ARM::thumbInst15<0,5>, &ARM::thumbInst15<0,5>, &ARM::thumbInst15<0,5>,
        &ARM::thumbInst15<0,6>, &ARM::thumbInst15<0,6>, &ARM::thumbInst15<0,6>, &ARM::thumbInst15<0,6>,
        &ARM::thumbInst15<0,7>, &ARM::thumbInst15<0,7>, &ARM::thumbInst15<0,7>, &ARM::thumbInst15<0,7>,
        &ARM::thumbInst15<1,0>, &ARM::thumbInst15<1,0>, &ARM::thumbInst15<1,0>, &ARM::thumbInst15<1,0>,
        &ARM::thumbInst15<1,1>, &ARM::thumbInst15<1,1>, &ARM::thumbInst15<1,1>, &ARM::thumbInst15<1,1>,
        &ARM::thumbInst15<1,2>, &ARM::thumbInst15<1,2>, &ARM::thumbInst15<1,2>, &ARM::thumbInst15<1,2>,
        &ARM::thumbInst15<1,3>, &ARM::thumbInst15<1,3>, &ARM::thumbInst15<1,3>, &ARM::thumbInst15<1,3>,
        &ARM::thumbInst15<1,4>, &ARM::thumbInst15<1,4>, &ARM::thumbInst15<1,4>, &ARM::thumbInst15<1,4>,
        &ARM::thumbInst15<1,5>, &ARM::thumbInst15<1,5>, &ARM::thumbInst15<1,5>, &ARM::thumbInst15<1,5>,
        &ARM::thumbInst15<1,6>, &ARM::thumbInst15<1,6>, &ARM::thumbInst15<1,6>, &ARM::thumbInst15<1,6>,
        &ARM::thumbInst15<1,7>, &ARM::thumbInst15<1,7>, &ARM::thumbInst15<1,7>, &ARM::thumbInst15<1,7>,

        /* THUMB.16 Conditional branch */
        &ARM::thumbInst16<0>,  &ARM::thumbInst16<0>,  &ARM::thumbInst16<0>,  &ARM::thumbInst16<0>,
        &ARM::thumbInst16<1>,  &ARM::thumbInst16<1>,  &ARM::thumbInst16<1>,  &ARM::thumbInst16<1>,
        &ARM::thumbInst16<2>,  &ARM::thumbInst16<2>,  &ARM::thumbInst16<2>,  &ARM::thumbInst16<2>,
        &ARM::thumbInst16<3>,  &ARM::thumbInst16<3>,  &ARM::thumbInst16<3>,  &ARM::thumbInst16<3>,
        &ARM::thumbInst16<4>,  &ARM::thumbInst16<4>,  &ARM::thumbInst16<4>,  &ARM::thumbInst16<4>,
        &ARM::thumbInst16<5>,  &ARM::thumbInst16<5>,  &ARM::thumbInst16<5>,  &ARM::thumbInst16<5>,
        &ARM::thumbInst16<6>,  &ARM::thumbInst16<6>,  &ARM::thumbInst16<6>,  &ARM::thumbInst16<6>,
        &ARM::thumbInst16<7>,  &ARM::thumbInst16<7>,  &ARM::thumbInst16<7>,  &ARM::thumbInst16<7>,
        &ARM::thumbInst16<8>,  &ARM::thumbInst16<8>,  &ARM::thumbInst16<8>,  &ARM::thumbInst16<8>,
        &ARM::thumbInst16<9>,  &ARM::thumbInst16<9>,  &ARM::thumbInst16<9>,  &ARM::thumbInst16<9>,
        &ARM::thumbInst16<10>, &ARM::thumbInst16<10>, &ARM::thumbInst16<10>, &ARM::thumbInst16<10>,
        &ARM::thumbInst16<11>, &ARM::thumbInst16<11>, &ARM::thumbInst16<11>, &ARM::thumbInst16<11>,
        &ARM::thumbInst16<12>, &ARM::thumbInst16<12>, &ARM::thumbInst16<12>, &ARM::thumbInst16<12>,
        &ARM::thumbInst16<13>, &ARM::thumbInst16<13>, &ARM::thumbInst16<13>, &ARM::thumbInst16<13>,
        &ARM::thumbInst16<14>, &ARM::thumbInst16<14>, &ARM::thumbInst16<14>, &ARM::thumbInst16<14>,

        /* THUMB.17 Software Interrupt */
        &ARM::thumbInst17, &ARM::thumbInst17, &ARM::thumbInst17, &ARM::thumbInst17,

        /* THUMB.18 Unconditional branch */
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,
        &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18, &ARM::thumbInst18,

        /* THUMB.19 Long branch with link */
        &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>,
        &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>,
        &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>,
        &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>,
        &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>,
        &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>,
        &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>,
        &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>, &ARM::thumbInst19<0>,
        &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>,
        &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>,
        &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>,
        &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>,
        &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>,
        &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>,
        &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>,
        &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>, &ARM::thumbInst19<1>
    };
}
