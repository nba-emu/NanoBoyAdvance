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

namespace GameBoyAdvance {
    
    #include "tables/op_thumb.hpp"

    template <int imm, int type>
    void ARM::thumb_1(u16 instruction) {
        // THUMB.1 Move shifted register
        int dst = instruction & 7;
        int src = (instruction >> 3) & 7;
        bool carry = ctx.cpsr & MASK_CFLAG;

        ctx.reg[dst] = ctx.reg[src];

        ApplyShift(type, ctx.reg[dst], imm, carry, true);

        // update carry, sign and zero flag
        SetCarryFlag(carry);
        UpdateSignFlag(ctx.reg[dst]);
        UpdateZeroFlag(ctx.reg[dst]);
    }

    template <bool immediate, bool subtract, int field3>
    void ARM::thumb_2(u16 instruction) {
        // THUMB.2 Add/subtract
        u32 operand, result;
        int dst = instruction & 7;
        int src = (instruction >> 3) & 7;

        // either a register or an immediate value
        operand = immediate ? field3 : ctx.reg[field3];

        if (subtract) {
            result = ctx.reg[src] - operand;

            SetCarryFlag(ctx.reg[src] >= operand);
            UpdateOverflowFlagSub(result, ctx.reg[src], operand);
        } else {
            u64 result_long = (u64)(ctx.reg[src]) + (u64)operand;

            result = (u32)result_long;
            SetCarryFlag(result_long & 0x100000000);
            UpdateOverflowFlagAdd(result, ctx.reg[src], operand);
        }

        UpdateSignFlag(result);
        UpdateZeroFlag(result);

        ctx.reg[dst] = result;
    }

    template <int op, int dst>
    void ARM::thumb_3(u16 instruction) {
        // THUMB.3 Move/compare/add/subtract immediate
        u32 result;
        u32 immediate_value = instruction & 0xFF;

        switch (op) {
        case 0b00: // MOV
            ctx.reg[dst] = immediate_value;
            UpdateSignFlag(0);
            UpdateZeroFlag(immediate_value);
            return;//important!

        case 0b01: // CMP
            result = ctx.reg[dst] - immediate_value;
            SetCarryFlag(ctx.reg[dst] >= immediate_value);
            UpdateOverflowFlagSub(result, ctx.reg[dst], immediate_value);
            break;
        case 0b10: { // ADD
            u64 result_long = (u64)ctx.reg[dst] + (u64)immediate_value;
            result = (u32)result_long;
            SetCarryFlag(result_long & 0x100000000);
            UpdateOverflowFlagAdd(result, ctx.reg[dst], immediate_value);
            ctx.reg[dst] = result;
            break;
        }
        case 0b11: // SUB
            result = ctx.reg[dst] - immediate_value;
            SetCarryFlag(ctx.reg[dst] >= immediate_value);
            UpdateOverflowFlagSub(result, ctx.reg[dst], immediate_value);
            ctx.reg[dst] = result;
            break;
        }

        UpdateSignFlag(result);
        UpdateZeroFlag(result);
    }

    template <int op>
    void ARM::thumb_4(u16 instruction)
    {
        // THUMB.4 ALU operations
        int dst = instruction & 7;
        int src = (instruction >> 3) & 7;

        switch (op) {
        case 0b0000: // AND
            ctx.reg[dst] &= ctx.reg[src];
            UpdateSignFlag(ctx.reg[dst]);
            UpdateZeroFlag(ctx.reg[dst]);
            break;
        case 0b0001: // EOR
            ctx.reg[dst] ^= ctx.reg[src];
            UpdateSignFlag(ctx.reg[dst]);
            UpdateZeroFlag(ctx.reg[dst]);
            break;
        case 0b0010: { // LSL
            u32 amount = ctx.reg[src];
            bool carry = ctx.cpsr & MASK_CFLAG;
            LogicalShiftLeft(ctx.reg[dst], amount, carry);
            SetCarryFlag(carry);
            UpdateSignFlag(ctx.reg[dst]);
            UpdateZeroFlag(ctx.reg[dst]);
            break;
        }
        case 0b0011: { // LSR
            u32 amount = ctx.reg[src];
            bool carry = ctx.cpsr & MASK_CFLAG;
            LogicalShiftRight(ctx.reg[dst], amount, carry, false);
            SetCarryFlag(carry);
            UpdateSignFlag(ctx.reg[dst]);
            UpdateZeroFlag(ctx.reg[dst]);
            break;
        }
        case 0b0100: { // ASR
            u32 amount = ctx.reg[src];
            bool carry = ctx.cpsr & MASK_CFLAG;
            ArithmeticShiftRight(ctx.reg[dst], amount, carry, false);
            SetCarryFlag(carry);
            UpdateSignFlag(ctx.reg[dst]);
            UpdateZeroFlag(ctx.reg[dst]);
            break;
        }
        case 0b0101: { // ADC
            int carry = (ctx.cpsr >> 29) & 1;
            u32 result = ctx.reg[dst] + ctx.reg[src] + carry;
            u64 result_long = (u64)(ctx.reg[dst]) + (u64)(ctx.reg[src]) + (u64)carry;
            SetCarryFlag(result_long & 0x100000000);
            UpdateOverflowFlagAdd(result, ctx.reg[dst], ctx.reg[src]);
            UpdateSignFlag(result);
            UpdateZeroFlag(result);
            ctx.reg[dst] = result;
            break;
        }
        case 0b0110: { // SBC
            int carry = (ctx.cpsr >> 29) & 1;
            u32 result = ctx.reg[dst] - ctx.reg[src] + carry - 1;
            SetCarryFlag(ctx.reg[dst] >= (ctx.reg[src] + carry - 1));
            UpdateOverflowFlagSub(result, ctx.reg[dst], ctx.reg[src]);
            UpdateSignFlag(result);
            UpdateZeroFlag(result);
            ctx.reg[dst] = result;
            break;
        }
        case 0b0111: { // ROR
            u32 amount = ctx.reg[src];
            bool carry = ctx.cpsr & MASK_CFLAG;
            RotateRight(ctx.reg[dst], amount, carry, false);
            SetCarryFlag(carry);
            UpdateSignFlag(ctx.reg[dst]);
            UpdateZeroFlag(ctx.reg[dst]);
            break;
        }
        case 0b1000: { // TST
            u32 result = ctx.reg[dst] & ctx.reg[src];
            UpdateSignFlag(result);
            UpdateZeroFlag(result);
            break;
        }
        case 0b1001: { // NEG
            u32 result = 0 - ctx.reg[src];
            SetCarryFlag(0 >= ctx.reg[src]);
            UpdateOverflowFlagSub(result, 0, ctx.reg[src]);
            UpdateSignFlag(result);
            UpdateZeroFlag(result);
            ctx.reg[dst] = result;
            break;
        }
        case 0b1010: { // CMP
            u32 result = ctx.reg[dst] - ctx.reg[src];
            SetCarryFlag(ctx.reg[dst] >= ctx.reg[src]);
            UpdateOverflowFlagSub(result, ctx.reg[dst], ctx.reg[src]);
            UpdateSignFlag(result);
            UpdateZeroFlag(result);
            break;
        }
        case 0b1011: { // CMN
            u32 result = ctx.reg[dst] + ctx.reg[src];
            u64 result_long = (u64)(ctx.reg[dst]) + (u64)(ctx.reg[src]);
            SetCarryFlag(result_long & 0x100000000);
            UpdateOverflowFlagAdd(result, ctx.reg[dst], ctx.reg[src]);
            UpdateSignFlag(result);
            UpdateZeroFlag(result);
            break;
        }
        case 0b1100: // ORR
            ctx.reg[dst] |= ctx.reg[src];
            UpdateSignFlag(ctx.reg[dst]);
            UpdateZeroFlag(ctx.reg[dst]);
            break;
        case 0b1101: // MUL
            // TODO: how to calc. the internal cycles?
            ctx.reg[dst] *= ctx.reg[src];
            UpdateSignFlag(ctx.reg[dst]);
            UpdateZeroFlag(ctx.reg[dst]);
            SetCarryFlag(false);
            break;
        case 0b1110: // BIC
            ctx.reg[dst] &= ~(ctx.reg[src]);
            UpdateSignFlag(ctx.reg[dst]);
            UpdateZeroFlag(ctx.reg[dst]);
            break;
        case 0b1111: // MVN
            ctx.reg[dst] = ~(ctx.reg[src]);
            UpdateSignFlag(ctx.reg[dst]);
            UpdateZeroFlag(ctx.reg[dst]);
            break;
        }
    }

    template <int op, bool high1, bool high2>
    void ARM::thumb_5(u16 instruction)
    {
        // THUMB.5 Hi register operations/branch exchange
        u32 operand;
        bool perform_check = true;
        int dst = instruction & 7;
        int src = (instruction >> 3) & 7;

        if (high1) dst |= 8;
        if (high2) src |= 8;

        operand = ctx.reg[src];
        if (src == 15) {
            operand &= ~1;
        }
            
        switch (op)
        {
        case 0: // ADD
            ctx.reg[dst] += operand;
            break;
        case 1: { // CMP
            u32 result = ctx.reg[dst] - operand;
            SetCarryFlag(ctx.reg[dst] >= operand);
            UpdateOverflowFlagSub(result, ctx.reg[dst], operand);
            UpdateSignFlag(result);
            UpdateZeroFlag(result);
            perform_check = false;
            break;
        }
        case 2: // MOV
            ctx.reg[dst] = operand;
            break;
        case 3: // BX
            if (operand & 1) {
                ctx.r15 = operand & ~1;
            } else {
                ctx.cpsr &= ~MASK_THUMB;
                ctx.r15 = operand & ~3;
            }
            ctx.pipe.do_flush = true;
            perform_check = false;
            break;
        }

        if (perform_check && dst == 15) {
            ctx.reg[dst] &= ~1;
            ctx.pipe.do_flush = true;
        }
    }

    template <int dst>
    void ARM::thumb_6(u16 instruction) {
        // THUMB.6 PC-relative load
        u32 imm     = instruction & 0xFF;
        u32 address = (ctx.r15 & ~2) + (imm << 2);

        ctx.reg[dst] = read_word(address);
    }

    template <int op, int off>
    void ARM::thumb_7(u16 instruction) {
        // THUMB.7 Load/store with register offset
        int dst = instruction & 7;
        int base = (instruction >> 3) & 7;
        u32 address = ctx.reg[base] + ctx.reg[off];

        switch (op) {
        case 0b00: // STR
            write_word(address, ctx.reg[dst]);
            break;
        case 0b01: // STRB
            bus_write_byte(address, (u8)ctx.reg[dst]);
            break;
        case 0b10: // LDR
            ctx.reg[dst] = read_word_rotated(address);
            break;
        case 0b11: // LDRB
            ctx.reg[dst] = bus_read_byte(address);
            break;
        }
    }

    template <int op, int off>
    void ARM::thumb_8(u16 instruction) {
        // THUMB.8 Load/store sign-extended byte/halfword
        int dst = instruction & 7;
        int base = (instruction >> 3) & 7;
        u32 address = ctx.reg[base] + ctx.reg[off];

        switch (op) {
        case 0b00: // STRH
            write_hword(address, ctx.reg[dst]);
            break;
        case 0b01: // LDSB
            ctx.reg[dst] = bus_read_byte(address);

            if (ctx.reg[dst] & 0x80) {
                ctx.reg[dst] |= 0xFFFFFF00;
            }
            break;
        case 0b10: // LDRH
            ctx.reg[dst] = read_hword(address);
            break;
        case 0b11: // LDSH
            ctx.reg[dst] = read_hword_signed(address);
            break;
        }
    }

    template <int op, int imm>
    void ARM::thumb_9(u16 instruction) {
        // THUMB.9 Load store with immediate offset
        int dst = instruction & 7;
        int base = (instruction >> 3) & 7;

        switch (op) {
        case 0b00: { // STR
            u32 address = ctx.reg[base] + (imm << 2);
            write_word(address, ctx.reg[dst]);
            break;
        }
        case 0b01: { // LDR
            u32 address = ctx.reg[base] + (imm << 2);
            ctx.reg[dst] = read_word_rotated(address);
            break;
        }
        case 0b10: { // STRB
            u32 address = ctx.reg[base] + imm;
            bus_write_byte(address, ctx.reg[dst]);
            break;
        }
        case 0b11: { // LDRB
            u32 address = ctx.reg[base] + imm;
            ctx.reg[dst] = bus_read_byte(address);
            break;
        }
        }
    }

    template <bool load, int imm>
    void ARM::thumb_10(u16 instruction) {
        // THUMB.10 Load/store halfword
        int dst     = instruction & 7;
        int base    = (instruction >> 3) & 7;
        u32 address = ctx.reg[base] + (imm << 1);

        if (load) {
            ctx.reg[dst] = read_hword(address);
        } else {
            write_hword(address, ctx.reg[dst]);
        }
    }

    template <bool load, int dst>
    void ARM::thumb_11(u16 instruction) {
        // THUMB.11 SP-relative load/store
        u32 imm     = instruction & 0xFF;
        u32 address = ctx.reg[13] + (imm << 2);

        if (load) {
            ctx.reg[dst] = read_word_rotated(address);
        } else {
            write_word(address, ctx.reg[dst]);
        }
    }

    template <bool stackptr, int dst>
    void ARM::thumb_12(u16 instruction) {
        // THUMB.12 Load address
        u32 address;
        u32 imm = (instruction & 0xFF) << 2;
        
        if (stackptr) {
            address = ctx.reg[13];
        } else {
            address = ctx.r15 & ~2;
        }

        ctx.reg[dst] = address + imm;
    }

    template <bool sub>
    void ARM::thumb_13(u16 instruction) {
        // THUMB.13 Add offset to stack pointer
        u32 imm = (instruction & 0x7F) << 2;

        ctx.reg[13] += sub ? -imm : imm;
    }

    template <bool pop, bool rbit>
    void ARM::thumb_14(u16 instruction) {
        // THUMB.14 push/pop registers
        u32 addr = ctx.reg[13];
        const int register_list = instruction & 0xFF;

        // hardware corner case. not sure if emulated correctly.
        if (!rbit && register_list == 0) {
            if (pop) {
                ctx.r15 = read_word(addr);
                ctx.pipe.do_flush   = true;
            } else {
                write_word(addr, ctx.r15);
            }
            ctx.reg[13] += pop ? 64 : -64;
            return;
        }

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
                    ctx.reg[i] = read_word(addr);
                } else {
                    write_word(addr, ctx.reg[i]);
                }
                addr += 4;
            }
        }

        if (rbit) {
            if (pop) {
                ctx.r15 = read_word(addr) & ~1;
                ctx.pipe.do_flush = true;
            } else {
                write_word(addr, ctx.reg[14]);
            }
            addr += 4;
        }

        if (pop) {
            ctx.reg[13] = addr;
        }
    }

    template <bool load, int base>
    void ARM::thumb_15(u16 instruction) {
        // THUMB.15 Multiple load/store
        bool write_back = true;
        u32 address = ctx.reg[base];
        int register_list = instruction & 0xFF;

        if (load) {
            if (register_list == 0) {
                ctx.r15 = read_word(address);
                ctx.pipe.do_flush = true;
                ctx.reg[base] += 64;
                return;
            }

            for (int i = 0; i <= 7; i++) {
                if (register_list & (1<<i)) {
                    ctx.reg[i] = read_word(address);
                    address += 4;
                }
            }

            if (write_back && (~register_list & (1<<base))) {
                ctx.reg[base] = address;
            }
        } else {
            int first_register = -1;

            if (register_list == 0) {
                write_word(address, ctx.r15);
                ctx.reg[base] += 64;
                return;
            }

            for (int i = 0; i <= 7; i++) {
                if (register_list & (1<<i)) {
                    if (first_register == -1) {
                        first_register = i;
                    }

                    if (i == base && i == first_register) {
                        write_word(ctx.reg[base], address);
                    } else {
                        write_word(ctx.reg[base], ctx.reg[i]);
                    }

                    ctx.reg[base] += 4;
                }
            }
        }
    }

    template <int cond>
    void ARM::thumb_16(u16 instruction) {
        // THUMB.16 Conditional branch
        if (CheckCondition(static_cast<Condition>(cond))) {
            u32 signed_immediate = instruction & 0xFF;

            // sign-extend the immediate value if neccessary
            if (signed_immediate & 0x80) {
                signed_immediate |= 0xFFFFFF00;
            }

            // update r15/pc and flush pipe
            ctx.r15 += (signed_immediate << 1);
            ctx.pipe.do_flush = true;
        }
    }

    void ARM::thumb_17(u16 instruction) {
        // THUMB.17 Software Interrupt
        u8 call_number = bus_read_byte(ctx.r15 - 4);
        
        if (!fake_swi) {
            // save return address and program status
            ctx.bank[BANK_SVC][BANK_R14] = ctx.r15 - 2;
            ctx.spsr[SPSR_SVC] = ctx.cpsr;

            // switch to SVC mode and disable interrupts
            SwitchMode(MODE_SVC);
            ctx.cpsr = (ctx.cpsr & ~MASK_THUMB) | MASK_IRQD;

            // jump to exception vector
            ctx.r15 = EXCPT_SWI;
            ctx.pipe.do_flush = true;
        } else {
            SoftwareInterrupt(call_number);
        }
    }

    void ARM::thumb_18(u16 instruction) {
        // THUMB.18 Unconditional branch
        u32 imm = (instruction & 0x3FF) << 1;

        // sign-extend r15/pc displacement
        if (instruction & 0x400) {
            imm |= 0xFFFFF800;
        }

        // update r15/pc and flush pipe
        ctx.r15 += imm;
        ctx.pipe.do_flush = true;
    }

    template <bool second_instruction>
    void ARM::thumb_19(u16 instruction) {
        // THUMB.19 Long branch with link.
        u32 imm = instruction & 0x7FF;

        if (second_instruction) {
            u32 temp_pc = ctx.r15 - 2;
            u32 value = ctx.reg[14] + (imm << 1);

            // update r15/pc
            value &= 0x7FFFFF;
            ctx.r15 &= ~0x7FFFFF;
            ctx.r15 |= value & ~1;

            // store return address and flush pipe.
            ctx.reg[14] = temp_pc | 1;
            ctx.pipe.do_flush = true;
        } else {
            ctx.reg[14] = ctx.r15 + (imm << 12);
        }
    }
}
