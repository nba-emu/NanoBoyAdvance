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
    
    #include "tables/op_arm.hpp"

    template <bool immediate, int opcode, bool _set_flags, int field4>
    void ARM::dataProcessingARM(u32 instruction) {
        u32 op1, op2;
        bool carry = ctx.cpsr & MASK_CFLAG;
        int reg_dst = (instruction >> 12) & 0xF;
        int reg_op1 = (instruction >> 16) & 0xF;
        bool set_flags = _set_flags;

        op1 = ctx.reg[reg_op1];

        if (immediate) {
            int imm = instruction & 0xFF;
            int amount = ((instruction >> 8) & 0xF) << 1;

            if (amount != 0) {
                carry = (imm >> (amount - 1)) & 1;
                op2 = (imm >> amount) | (imm << (32 - amount));
            } else {
                op2 = imm;
            }
        } else {
            u32 amount;
            int reg_op2 = instruction & 0xF;
            int shift_type = (field4 >> 1) & 3;
            bool shift_immediate = (field4 & 1) ? false : true;

            op2 = ctx.reg[reg_op2];

            if (!shift_immediate) {
                amount = ctx.reg[(instruction >> 8) & 0xF];
                
                if (reg_op1 == 15) op1 += 4;
                if (reg_op2 == 15) op2 += 4;
                //cycles++;
            } else {
                amount = (instruction >> 7) & 0x1F;
            }

            applyShift(shift_type, op2, amount, carry, shift_immediate);
        }

        if (reg_dst == 15) {
            if (set_flags) {
                u32 spsr = *ctx.p_spsr;
                switchMode(static_cast<Mode>(spsr & MASK_MODE));
                ctx.cpsr = spsr;
                set_flags = false;
            }
            ctx.pipe.do_flush = true;
        }

        switch (opcode) {
        case 0b0000: {
            // Bitwise AND (AND)
            u32 result = op1 & op2;

            if (set_flags) {
                updateSignFlag(result);
                updateZeroFlag(result);
                updateCarryFlag(carry);
            }

            ctx.reg[reg_dst] = result;
            break;
        }
        case 0b0001: {
            // Bitwise EXOR (EOR)
            u32 result = op1 ^ op2;

            if (set_flags) {
                updateSignFlag(result);
                updateZeroFlag(result);
                updateCarryFlag(carry);
            }

            ctx.reg[reg_dst] = result;
            break;
        }
        case 0b0010: {
            // Subtraction (SUB)
            u32 result = op1 - op2;

            if (set_flags) {
                updateCarryFlag(op1 >= op2);
                updateOverflowFlagSub(result, op1, op2);
                updateSignFlag(result);
                updateZeroFlag(result);
            }

            ctx.reg[reg_dst] = result;
            break;
        }
        case 0b0011: {
            // Reverse subtraction (RSB)
            u32 result = op2 - op1;

            if (set_flags) {
                updateCarryFlag(op2 >= op1);
                updateOverflowFlagSub(result, op2, op1);
                updateSignFlag(result);
                updateZeroFlag(result);
            }

            ctx.reg[reg_dst] = result;
            break;
        }
        case 0b0100: {
            // Addition (ADD)
            u32 result = op1 + op2;

            if (set_flags) {
                u64 result_long = (u64)op1 + (u64)op2;

                updateCarryFlag(result_long & 0x100000000);
                updateOverflowFlagAdd(result, op1, op2);
                updateSignFlag(result);
                updateZeroFlag(result);
            }

            ctx.reg[reg_dst] = result;
            break;
        }
        case 0b0101: {
            // Addition with Carry
            int carry2 = (ctx.cpsr >> 29) & 1;
            u32 result = op1 + op2 + carry2;

            if (set_flags) {
                u64 result_long = (u64)op1 + (u64)op2 + (u64)carry2;

                updateCarryFlag(result_long & 0x100000000);
                updateOverflowFlagAdd(result, op1, op2);
                updateSignFlag(result);
                updateZeroFlag(result);
            }

            ctx.reg[reg_dst] = result;
            break;
        }
        case 0b0110: {
            // Subtraction with Carry
            int carry2 = (ctx.cpsr >> 29) & 1;
            u32 result = op1 - op2 + carry2 - 1;

            if (set_flags) {
                updateCarryFlag(op1 >= (op2 + carry2 - 1));
                updateOverflowFlagSub(result, op1, op2);
                updateSignFlag(result);
                updateZeroFlag(result);
            }

            ctx.reg[reg_dst] = result;
            break;
        }
        case 0b0111: {
            // Reverse Substraction with Carry
            int carry2 = (ctx.cpsr >> 29) & 1;
            u32 result = op2 - op1 + carry2 - 1;

            if (set_flags) {
                updateCarryFlag(op2 >= (op1 + carry2 - 1));
                updateOverflowFlagSub(result, op2, op1);
                updateSignFlag(result);
                updateZeroFlag(result);
            }

            ctx.reg[reg_dst] = result;
            break;
        }
        case 0b1000: {
            // Bitwise AND flags only (TST)
            u32 result = op1 & op2;

            updateSignFlag(result);
            updateZeroFlag(result);
            updateCarryFlag(carry);
            break;
        }
        case 0b1001: {
            // Bitwise EXOR flags only (TEQ)
            u32 result = op1 ^ op2;

            updateSignFlag(result);
            updateZeroFlag(result);
            updateCarryFlag(carry);
            break;
        }
        case 0b1010: {
            // Subtraction flags only (CMP)
            u32 result = op1 - op2;

            updateCarryFlag(op1 >= op2);
            updateOverflowFlagSub(result, op1, op2);
            updateSignFlag(result);
            updateZeroFlag(result);
            break;
        }
        case 0b1011: {
            // Addition flags only (CMN)
            u32 result = op1 + op2;
            u64 result_long = (u64)op1 + (u64)op2;

            updateCarryFlag(result_long & 0x100000000);
            updateOverflowFlagAdd(result, op1, op2);
            updateSignFlag(result);
            updateZeroFlag(result);
            break;
        }
        case 0b1100: {
            // Bitwise OR (ORR)
            u32 result = op1 | op2;

            if (set_flags) {
                updateSignFlag(result);
                updateZeroFlag(result);
                updateCarryFlag(carry);
            }

            ctx.reg[reg_dst] = result;
            break;
        }
        case 0b1101: {
            // Move into register (MOV)
            if (set_flags) {
                updateSignFlag(op2);
                updateZeroFlag(op2);
                updateCarryFlag(carry);
            }

            ctx.reg[reg_dst] = op2;
            break;
        }
        case 0b1110: {
            // Bit Clear (BIC)
            u32 result = op1 & ~op2;

            if (set_flags) {
                updateSignFlag(result);
                updateZeroFlag(result);
                updateCarryFlag(carry);
            }

            ctx.reg[reg_dst] = result;
            break;
        }
        case 0b1111: {
            // Move into register negated (MVN)
            u32 not_op2 = ~op2;

            if (set_flags) {
                updateSignFlag(not_op2);
                updateZeroFlag(not_op2);
                updateCarryFlag(carry);
            }

            ctx.reg[reg_dst] = not_op2;
            break;
        }
        }
    }

    template <bool immediate, bool use_spsr, bool to_status>
    void ARM::statusTransferARM(u32 instruction) {
        if (to_status) {
            u32 op;
            u32 mask = 0;

            // create mask based on fsxc-bits.
            if (instruction & (1 << 16)) mask |= 0x000000FF;
            if (instruction & (1 << 17)) mask |= 0x0000FF00;
            if (instruction & (1 << 18)) mask |= 0x00FF0000;
            if (instruction & (1 << 19)) mask |= 0xFF000000;

            // decode source operand
            if (immediate) {
                int imm = instruction & 0xFF;
                int amount = ((instruction >> 8) & 0xF) << 1;
                op = (imm >> amount) | (imm << (32 - amount));
            } else {
                op = ctx.reg[instruction & 0xF];
            }
            
            u32 value = op & mask;

            // write to cpsr or spsr
            if (!use_spsr) {
                // only switch mode if it actually gets written to
                if (mask & 0xFF) {
                    switchMode(static_cast<Mode>(value & MASK_MODE));
                }
                ctx.cpsr = (ctx.cpsr & ~mask) | value;
            } else {
                *ctx.p_spsr = (*ctx.p_spsr & ~mask) | value;
            }
        } else {
            int dst = (instruction >> 12) & 0xF;
            
            ctx.reg[dst] = use_spsr ? *ctx.p_spsr : ctx.cpsr;
        }
    }

    template <bool accumulate, bool set_flags>
    void ARM::multiplyARM(u32 instruction) {
        u32 result;
        int op1 = instruction & 0xF;
        int op2 = (instruction >> 8) & 0xF;
        int dst = (instruction >> 16) & 0xF;

        result = ctx.reg[op1] * ctx.reg[op2];

        if (accumulate) {
            int op3 = (instruction >> 12) & 0xF;
            result += ctx.reg[op3];
        }

        if (set_flags) {
            updateSignFlag(result);
            updateZeroFlag(result);
        }

        ctx.reg[dst] = result;
    }

    template <bool sign_extend, bool accumulate, bool set_flags>
    void ARM::multiplyLongARM(u32 instruction) {
        s64 result;
        int op1 = instruction & 0xF;
        int op2 = (instruction >> 8) & 0xF;
        int dst_lo = (instruction >> 12) & 0xF;
        int dst_hi = (instruction >> 16) & 0xF;

        if (sign_extend) {
            s64 op1_value = ctx.reg[op1];
            s64 op2_value = ctx.reg[op2];

            // sign-extend operands
            if (op1_value & 0x80000000) op1_value |= 0xFFFFFFFF00000000;
            if (op2_value & 0x80000000) op2_value |= 0xFFFFFFFF00000000;

            result = op1_value * op2_value;
        } else {
            u64 uresult = (u64)ctx.reg[op1] * (u64)ctx.reg[op2];
            result = uresult;
        }

        if (accumulate) {
            s64 value = ctx.reg[dst_hi];

            // workaround required by x86.
            value <<= 16;
            value <<= 16;
            value |= ctx.reg[dst_lo];

            result += value;
        }

        u32 result_hi = result >> 32;

        ctx.reg[dst_lo] = result & 0xFFFFFFFF;
        ctx.reg[dst_hi] = result_hi;

        if (set_flags) {
            updateSignFlag(result_hi);
            updateZeroFlag(result);
        }
    }

    template <bool swap_byte>
    void ARM::singleDataSwapARM(u32 instruction) {
        u32 tmp;
        int src = instruction & 0xF;
        int dst = (instruction >> 12) & 0xF;
        int base = (instruction >> 16) & 0xF;

        if (swap_byte) {
            tmp = read8(ctx.reg[base], M_NONE);
            write8(ctx.reg[base], (u8)ctx.reg[src], M_NONE);
        } else {
            tmp = read32(ctx.reg[base], M_ROTATE);
            write32(ctx.reg[base], ctx.reg[src], M_NONE);
        }

        ctx.reg[dst] = tmp;
    }

    void ARM::branchExchangeARM(u32 instruction) {
        u32 addr = ctx.reg[instruction & 0xF];

        ctx.pipe.do_flush = true;

        if (addr & 1) {
            ctx.r15 = addr & ~1;
            ctx.cpsr |= MASK_THUMB;
        } else {
            ctx.r15 = addr & ~3;
        }
    }

    template <bool pre_indexed, bool base_increment, bool immediate, bool write_back, bool load, int opcode>
    void ARM::halfwordSignedTransferARM(u32 instruction) {
        u32 off;
        int dst = (instruction >> 12) & 0xF;
        int base = (instruction >> 16) & 0xF;
        u32 addr = ctx.reg[base];

        if (immediate) {
            off = (instruction & 0xF) | ((instruction >> 4) & 0xF0);
        } else {
            off = ctx.reg[instruction & 0xF];
        }

        if (pre_indexed) {
            addr += base_increment ? off : -off;
        }

        switch (opcode) {
        case 1:
            // load/store halfword
            if (load) {
                ctx.reg[dst] = read16(addr, M_ROTATE);
            } else {
                u32 value = ctx.reg[dst];

                // r15 is $+12 instead of $+8 because of internal prefetch.
                if (dst == 15) {
                    value += 4;
                }

                write16(addr, value, M_NONE);
            }
            break;
        case 2: {
            // load signed byte
            ctx.reg[dst] = read8(addr, M_SIGNED);
            break;
        }
        case 3: {
            // load signed halfword
            ctx.reg[dst] = read16(addr, M_SIGNED);
            break;
        }
        }

        if ((write_back || !pre_indexed) && base != dst) {
            if (!pre_indexed) {
                addr += base_increment ? off : -off;
            }
            ctx.reg[base] = addr;
        }
    }

    template <bool immediate, bool pre_indexed, bool base_increment, bool byte, bool write_back, bool load>
    void ARM::singleTransferARM(u32 instruction) {
        u32 off;
        Mode old_mode;
        int dst  = (instruction >> 12) & 0xF;
        int base = (instruction >> 16) & 0xF;
        u32 addr = ctx.reg[base];

        // post-indexing implicitly performs a write back.
        // in that case W-bit indicates wether user-mode register access should be enforced.
        if (!pre_indexed && write_back) {
            old_mode = static_cast<Mode>(ctx.cpsr & MASK_MODE);
            switchMode(MODE_USR);
        }

        // get address offset.
        if (immediate) {
            off = instruction & 0xFFF;
        } else {
            bool carry = ctx.cpsr & MASK_CFLAG;
            int shift = (instruction >> 5) & 3;
            u32 amount = (instruction >> 7) & 0x1F;

            off = ctx.reg[instruction & 0xF];
            applyShift(shift, off, amount, carry, true);
        }

        if (pre_indexed) {
            addr += base_increment ? off : -off;
        }

        if (load) {
            ctx.reg[dst] = byte ? read8(addr, M_NONE) : read32(addr, M_ROTATE);
            
            // writes to r15 require a pipeline flush.
            if (dst == 15) {
                ctx.pipe.do_flush = true;
            }
        } else {
            u32 value = ctx.reg[dst];

            // r15 is $+12 now due to internal prefetch cycle.
            if (dst == 15) {
                value += 4;
            }

            if (byte) {
                write8(addr, (u8)value, M_NONE);
            } else {
                write32(addr, value, M_NONE);
            }
        }

        // writeback operation may not overwrite the destination register.
        if (base != dst) {
            if (!pre_indexed) {
                ctx.reg[base] += base_increment ? off : -off;

                // if user-mode was enforced, return to previous mode.
                if (write_back) {
                    switchMode(old_mode);
                }
            }
            else if (write_back)
            {
                ctx.reg[base] = addr;
            }
        }
    }

    void ARM::undefinedInstARM(u32 instruction) {
        // save return address and program status
        ctx.bank[BANK_SVC][BANK_R14] = ctx.r15 - 4;
        ctx.spsr[SPSR_SVC] = ctx.cpsr;

        // switch to UND mode and disable interrupts
        switchMode(MODE_UND);
        ctx.cpsr |= MASK_IRQD;

        // jump to exception vector
        ctx.r15 = EXCPT_UNDEFINED;
        ctx.pipe.do_flush = true;
    }

    template <bool _pre_indexed, bool base_increment, bool user_mode, bool _write_back, bool load>
    void ARM::blockTransferARM(u32 instruction) {
        int first_register;
        int register_count = 0;
        int register_list = instruction & 0xFFFF;
        bool transfer_r15 = register_list & (1 << 15);
        bool pre_indexed = _pre_indexed;
        bool write_back = _write_back;

        int base = (instruction >> 16) & 0xF;

        Mode old_mode;
        bool mode_switched = false;

        /*// hardware corner case. not sure if emulated correctly.
        if (register_list == 0) {
            if (load) {
                ctx.r15 = read32(ctx.reg[base], M_NONE);
                ctx.pipe.do_flush = true;
            } else {
                write32(ctx.reg[base], ctx.r15, M_NONE);
            }
            ctx.reg[base] += base_increment ? 64 : -64;
            return;
        }*/

        if (user_mode && (!load || !transfer_r15)) {
            old_mode = static_cast<Mode>(ctx.cpsr & MASK_MODE);
            switchMode(MODE_USR);
            mode_switched = true;
        }

        // find first register in list and also count them.
        for (int i = 15; i >= 0; i--) {
            if (~register_list & (1 << i)) {
                continue;
            }

            first_register = i;
            register_count++;
        }

        u32 addr = ctx.reg[base];
        u32 addr_old = addr;

        // instruction internally works with incrementing addresses.
        if (!base_increment) {
            addr -= register_count * 4;
            ctx.reg[base] = addr;
            write_back = false;
            pre_indexed = !pre_indexed;
        }

        // process register list starting with the lowest address and register.
        for (int i = first_register; i < 16; i++) {
            if (~register_list & (1 << i)) {
                continue;
            }

            if (pre_indexed) { 
                addr += 4;
            }

            if (load) {
                if (i == base) {
                    write_back = false;
                }

                ctx.reg[i] = read32(addr, M_NONE);

                if (i == 15) {
                    if (user_mode) {
                        u32 spsr = *ctx.p_spsr;
                        switchMode(static_cast<Mode>(spsr & MASK_MODE));
                        ctx.cpsr = spsr;
                    }
                    ctx.pipe.do_flush = true;
                }
            } else {
                if (i == first_register && i == base) {
                    write32(addr, addr_old, M_NONE);
                } else {
                    write32(addr, ctx.reg[i], M_NONE);
                }
            }

            if (!pre_indexed) {
                addr += 4;
            }

            if (write_back) { 
                ctx.reg[base] = addr; 
            }
        }

        if (mode_switched) {
            switchMode(old_mode);
        }
    }

    template <bool link>
    void ARM::branchARM(u32 instruction) {
        u32 off = instruction & 0xFFFFFF;

        if (off & 0x800000) {
            off |= 0xFF000000;
        }
        
        if (link) { 
            ctx.reg[14] = ctx.r15 - 4;
        }
        
        ctx.r15 += off << 2;
        ctx.pipe.do_flush = true;
    }

    void ARM::swiARM(u32 instruction) {
        u32 call_number = read8(ctx.r15 - 6, M_NONE);

        if (!fake_swi) {
            // save return address and program status
            ctx.bank[BANK_SVC][BANK_R14] = ctx.r15 - 4;
            ctx.spsr[SPSR_SVC] = ctx.cpsr;

            // switch to SVC mode and disable interrupts
            switchMode(MODE_SVC);
            ctx.cpsr |= MASK_IRQD;

            // jump to exception vector
            ctx.r15 = EXCPT_SWI;
            ctx.pipe.do_flush = true;
        } else {
            handleSWI(call_number);
        }
    }
}
