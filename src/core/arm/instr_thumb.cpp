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

#define ADVANCE_PC \
    if (++ctx.pipe.index == 3) ctx.pipe.index = 0;\
    ctx.r15 += 2;

#define REFILL_PIPELINE_A \
    ctx.pipe.index     = 0;\
    ctx.pipe.opcode[0] = busRead32(ctx.r15,     M_NONSEQ);\
    ctx.pipe.opcode[1] = busRead32(ctx.r15 + 4, M_SEQ);\
    ctx.r15 += 8;

#define REFILL_PIPELINE_T \
    ctx.pipe.index     = 0;\
    ctx.pipe.opcode[0] = busRead16(ctx.r15,     M_NONSEQ);\
    ctx.pipe.opcode[1] = busRead16(ctx.r15 + 2, M_SEQ);\
    ctx.r15 += 4;

namespace GameBoyAdvance {

    #include "tables/op_thumb.hpp"

    template <int imm, int type>
    void ARM::thumbInst1(u16 instruction) {
        // THUMB.1 Move shifted register
        int dst = instruction & 7;
        int src = (instruction >> 3) & 7;
        bool carry = ctx.cpsr & MASK_CFLAG;

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
        u32 operand, result;
        int dst = instruction & 7;
        int src = (instruction >> 3) & 7;

        // either a register or an immediate value
        operand = immediate ? field3 : ctx.reg[field3];

        if (subtract) {
            result = ctx.reg[src] - operand;

            updateCarryFlag(ctx.reg[src] >= operand);
            updateOverflowFlagSub(result, ctx.reg[src], operand);
        } else {
            u64 result_long = (u64)(ctx.reg[src]) + (u64)operand;

            result = (u32)result_long;
            updateCarryFlag(result_long & 0x100000000);
            updateOverflowFlagAdd(result, ctx.reg[src], operand);
        }

        updateSignFlag(result);
        updateZeroFlag(result);

        ctx.reg[dst] = result;

        ADVANCE_PC;
    }

    template <int op, int dst>
    void ARM::thumbInst3(u16 instruction) {
        // THUMB.3 Move/compare/add/subtract immediate
        u32 result;
        u32 immediate_value = instruction & 0xFF;

        switch (op) {
        case 0b00: // MOV
            ctx.reg[dst] = immediate_value;
            updateSignFlag(0);
            updateZeroFlag(immediate_value);

            //TODO: handle this case in a more straight-forward manner?
            ADVANCE_PC;
            return;//important!

        case 0b01: // CMP
            result = ctx.reg[dst] - immediate_value;
            updateCarryFlag(ctx.reg[dst] >= immediate_value);
            updateOverflowFlagSub(result, ctx.reg[dst], immediate_value);
            break;
        case 0b10: { // ADD
            u64 result_long = (u64)ctx.reg[dst] + (u64)immediate_value;
            result = (u32)result_long;
            updateCarryFlag(result_long & 0x100000000);
            updateOverflowFlagAdd(result, ctx.reg[dst], immediate_value);
            ctx.reg[dst] = result;
            break;
        }
        case 0b11: // SUB
            result = ctx.reg[dst] - immediate_value;
            updateCarryFlag(ctx.reg[dst] >= immediate_value);
            updateOverflowFlagSub(result, ctx.reg[dst], immediate_value);
            ctx.reg[dst] = result;
            break;
        }

        updateSignFlag(result);
        updateZeroFlag(result);

        ADVANCE_PC;
    }

    template <int op>
    void ARM::thumbInst4(u16 instruction) {
        // THUMB.4 ALU operations
        int dst = instruction & 7;
        int src = (instruction >> 3) & 7;

        switch (op) {
        case 0b0000: // AND
            ctx.reg[dst] &= ctx.reg[src];
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            break;
        case 0b0001: // EOR
            ctx.reg[dst] ^= ctx.reg[src];
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            break;
        case 0b0010: { // LSL
            u32 amount = ctx.reg[src];
            bool carry = ctx.cpsr & MASK_CFLAG;
            shiftLSL(ctx.reg[dst], amount, carry);
            updateCarryFlag(carry);
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            break;
        }
        case 0b0011: { // LSR
            u32 amount = ctx.reg[src];
            bool carry = ctx.cpsr & MASK_CFLAG;
            shiftLSR(ctx.reg[dst], amount, carry, false);
            updateCarryFlag(carry);
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            break;
        }
        case 0b0100: { // ASR
            u32 amount = ctx.reg[src];
            bool carry = ctx.cpsr & MASK_CFLAG;
            shiftASR(ctx.reg[dst], amount, carry, false);
            updateCarryFlag(carry);
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            break;
        }
        case 0b0101: { // ADC
            int carry = (ctx.cpsr >> 29) & 1;
            u32 result = ctx.reg[dst] + ctx.reg[src] + carry;
            u64 result_long = (u64)(ctx.reg[dst]) + (u64)(ctx.reg[src]) + (u64)carry;
            updateCarryFlag(result_long & 0x100000000);
            updateOverflowFlagAdd(result, ctx.reg[dst], ctx.reg[src]);
            updateSignFlag(result);
            updateZeroFlag(result);
            ctx.reg[dst] = result;
            break;
        }
        case 0b0110: { // SBC
            int carry = (ctx.cpsr >> 29) & 1;
            u32 result = ctx.reg[dst] - ctx.reg[src] + carry - 1;
            updateCarryFlag(ctx.reg[dst] >= (ctx.reg[src] + carry - 1));
            updateOverflowFlagSub(result, ctx.reg[dst], ctx.reg[src]);
            updateSignFlag(result);
            updateZeroFlag(result);
            ctx.reg[dst] = result;
            break;
        }
        case 0b0111: { // ROR
            u32 amount = ctx.reg[src];
            bool carry = ctx.cpsr & MASK_CFLAG;
            shiftROR(ctx.reg[dst], amount, carry, false);
            updateCarryFlag(carry);
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            break;
        }
        case 0b1000: { // TST
            u32 result = ctx.reg[dst] & ctx.reg[src];
            updateSignFlag(result);
            updateZeroFlag(result);
            break;
        }
        case 0b1001: { // NEG
            u32 result = 0 - ctx.reg[src];
            updateCarryFlag(0 >= ctx.reg[src]);
            updateOverflowFlagSub(result, 0, ctx.reg[src]);
            updateSignFlag(result);
            updateZeroFlag(result);
            ctx.reg[dst] = result;
            break;
        }
        case 0b1010: { // CMP
            u32 result = ctx.reg[dst] - ctx.reg[src];
            updateCarryFlag(ctx.reg[dst] >= ctx.reg[src]);
            updateOverflowFlagSub(result, ctx.reg[dst], ctx.reg[src]);
            updateSignFlag(result);
            updateZeroFlag(result);
            break;
        }
        case 0b1011: { // CMN
            u32 result = ctx.reg[dst] + ctx.reg[src];
            u64 result_long = (u64)(ctx.reg[dst]) + (u64)(ctx.reg[src]);
            updateCarryFlag(result_long & 0x100000000);
            updateOverflowFlagAdd(result, ctx.reg[dst], ctx.reg[src]);
            updateSignFlag(result);
            updateZeroFlag(result);
            break;
        }
        case 0b1100: // ORR
            ctx.reg[dst] |= ctx.reg[src];
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            break;
        case 0b1101: // MUL
            // TODO: how to calc. the internal cycles?
            ctx.reg[dst] *= ctx.reg[src];
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            updateCarryFlag(false);
            break;
        case 0b1110: // BIC
            ctx.reg[dst] &= ~(ctx.reg[src]);
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            break;
        case 0b1111: // MVN
            ctx.reg[dst] = ~(ctx.reg[src]);
            updateSignFlag(ctx.reg[dst]);
            updateZeroFlag(ctx.reg[dst]);
            break;
        }

        ADVANCE_PC;
    }

    template <int op, bool high1, bool high2>
    void ARM::thumbInst5(u16 instruction) {
        // THUMB.5 Hi register operations/branch exchange
        u32 operand;
        bool perform_check = true;
        int dst = instruction & 7;
        int src = (instruction >> 3) & 7;

        if (high1) dst |= 8;
        if (high2) src |= 8;

        operand = ctx.reg[src];
        if (high2 && src == 15) {
            operand &= ~1;
        }

        switch (op) {
        case 0: // ADD
            ctx.reg[dst] += operand;
            break;
        case 1: { // CMP
            u32 result = ctx.reg[dst] - operand;
            updateCarryFlag(ctx.reg[dst] >= operand);
            updateOverflowFlagSub(result, ctx.reg[dst], operand);
            updateSignFlag(result);
            updateZeroFlag(result);
            perform_check = false;
            break;
        }
        case 2: // MOV
            ctx.reg[dst] = operand;
            break;
        case 3: // BX
            if (operand & 1) {
                ctx.r15 = operand & ~1;
                REFILL_PIPELINE_T;
            } else {
                ctx.cpsr &= ~MASK_THUMB;
                ctx.r15 = operand & ~3;
                REFILL_PIPELINE_A;
            }
            return; // do not advance pc
        }

        if (perform_check && dst == 15) {
            ctx.reg[dst] &= ~1; // TODO: use r15 directly
            REFILL_PIPELINE_T;
            return; // do not advance pc
        }

        ADVANCE_PC;
    }

    template <int dst>
    void ARM::thumbInst6(u16 instruction) {
        // THUMB.6 PC-relative load
        u32 imm     = instruction & 0xFF;
        u32 address = (ctx.r15 & ~2) + (imm << 2);

        ctx.reg[dst] = read32(address, M_NONE);

        ADVANCE_PC;
    }

    template <int op, int off>
    void ARM::thumbInst7(u16 instruction) {
        // THUMB.7 Load/store with register offset
        int dst = instruction & 7;
        int base = (instruction >> 3) & 7;
        u32 address = ctx.reg[base] + ctx.reg[off];

        switch (op) {
        case 0b00: // STR
            write32(address, ctx.reg[dst], M_NONE);
            break;
        case 0b01: // STRB
            write8(address, (u8)ctx.reg[dst], M_NONE);
            break;
        case 0b10: // LDR
            ctx.reg[dst] = read32(address, M_ROTATE);
            break;
        case 0b11: // LDRB
            ctx.reg[dst] = read8(address, M_NONE);
            break;
        }

        ADVANCE_PC;
    }

    template <int op, int off>
    void ARM::thumbInst8(u16 instruction) {
        // THUMB.8 Load/store sign-extended byte/halfword
        int dst = instruction & 7;
        int base = (instruction >> 3) & 7;
        u32 address = ctx.reg[base] + ctx.reg[off];

        switch (op) {
        case 0b00: // STRH
            write16(address, ctx.reg[dst], M_NONE);
            break;
        case 0b01: // LDSB
            ctx.reg[dst] = read8(address, M_SIGNED);
            break;
        case 0b10: // LDRH
            ctx.reg[dst] = read16(address, M_ROTATE);
            break;
        case 0b11: // LDSH
            ctx.reg[dst] = read16(address, M_SIGNED);
            break;
        }

        ADVANCE_PC;
    }

    template <int op, int imm>
    void ARM::thumbInst9(u16 instruction) {
        // THUMB.9 Load store with immediate offset
        int dst = instruction & 7;
        int base = (instruction >> 3) & 7;

        switch (op) {
        case 0b00: { // STR
            u32 address = ctx.reg[base] + (imm << 2);
            write32(address, ctx.reg[dst], M_NONE);
            break;
        }
        case 0b01: { // LDR
            u32 address = ctx.reg[base] + (imm << 2);
            ctx.reg[dst] = read32(address, M_ROTATE);
            break;
        }
        case 0b10: { // STRB
            u32 address = ctx.reg[base] + imm;
            write8(address, ctx.reg[dst], M_NONE);
            break;
        }
        case 0b11: { // LDRB
            u32 address = ctx.reg[base] + imm;
            ctx.reg[dst] = read8(address, M_NONE);
            break;
        }
        }

        ADVANCE_PC;
    }

    template <bool load, int imm>
    void ARM::thumbInst10(u16 instruction) {
        // THUMB.10 Load/store halfword
        int dst     = instruction & 7;
        int base    = (instruction >> 3) & 7;
        u32 address = ctx.reg[base] + (imm << 1);

        if (load) {
            ctx.reg[dst] = read16(address, M_ROTATE);
        } else {
            write16(address, ctx.reg[dst], M_NONE);
        }

        ADVANCE_PC;
    }

    template <bool load, int dst>
    void ARM::thumbInst11(u16 instruction) {
        // THUMB.11 SP-relative load/store
        u32 imm     = instruction & 0xFF;
        u32 address = ctx.reg[13] + (imm << 2);

        if (load) {
            ctx.reg[dst] = read32(address, M_NONSEQ | M_ROTATE);
        } else {
            write32(address, ctx.reg[dst], M_NONSEQ);
        }

        ADVANCE_PC;
    }

    template <bool stackptr, int dst>
    void ARM::thumbInst12(u16 instruction) {
        // THUMB.12 Load address
        u32 address;
        u32 imm = (instruction & 0xFF) << 2;

        if (stackptr) {
            address = ctx.reg[13];
        } else {
            address = ctx.r15 & ~2;
        }

        ctx.reg[dst] = address + imm;
        ADVANCE_PC;
    }

    template <bool sub>
    void ARM::thumbInst13(u16 instruction) {
        // THUMB.13 Add offset to stack pointer
        u32 imm = (instruction & 0x7F) << 2;

        ctx.reg[13] += sub ? -imm : imm;
        ADVANCE_PC;
    }

    template <bool pop, bool rbit>
    void ARM::thumbInst14(u16 instruction) {
        // THUMB.14 push/pop registers
        u32 addr = ctx.reg[13];
        const int register_list = instruction & 0xFF;

        // TODO: emulate empty register list

        if (!pop) {
            int register_count = 0;

            for (int i = 0; i <= 7; i++) {
                if (register_list & (1 << i)) {
                    register_count++;
                }
            }

            if (rbit) {
                register_count++;
            }

            addr -= register_count << 2;
            ctx.reg[13] = addr;
        }

        // perform load/store multiple
        for (int i = 0; i <= 7; i++) {
            if (register_list & (1 << i)) {
                if (pop) {
                    ctx.reg[i] = read32(addr, M_NONE);
                } else {
                    write32(addr, ctx.reg[i], M_NONE);
                }
                addr += 4;
            }
        }

        if (rbit) {
            if (pop) {
                ctx.r15 = read32(addr, M_NONE) & ~1;

                // refill pipe and fast return, no pc advance
                REFILL_PIPELINE_T;
                ctx.reg[13] = addr + 4;
                return;
            } else {
                write32(addr, ctx.reg[14], M_NONE);
            }
            addr += 4;
        }

        if (pop) {
            ctx.reg[13] = addr;
        }

        ADVANCE_PC;
    }

    template <bool load, int base>
    void ARM::thumbInst15(u16 instruction) {
        // THUMB.15 Multiple load/store
        bool write_back = true;
        u32 address = ctx.reg[base];
        int register_list = instruction & 0xFF;

        // TODO: emulate empty register list

        if (load) {
            for (int i = 0; i <= 7; i++) {
                if (register_list & (1<<i)) {
                    ctx.reg[i] = read32(address, M_NONE);
                    address += 4;
                }
            }

            if (write_back && (~register_list & (1<<base))) {
                ctx.reg[base] = address;
            }
        } else {
            int first_register = -1;

            for (int i = 0; i <= 7; i++) {
                if (register_list & (1<<i)) {
                    if (first_register == -1) {
                        first_register = i;
                    }

                    if (i == base && i == first_register) {
                        write32(ctx.reg[base], address, M_NONE);
                    } else {
                        write32(ctx.reg[base], ctx.reg[i], M_NONE);
                    }

                    ctx.reg[base] += 4;
                }
            }
        }

        ADVANCE_PC;
    }

    template <int cond>
    void ARM::thumbInst16(u16 instruction) {
        // THUMB.16 Conditional branch
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

        if (!fake_swi) {
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
}
