///////////////////////////////////////////////////////////////////////////////////
//
//  ARMigo is an ARM7TDMI-S emulator/interpreter.
//  Copyright (C) 2017 Frederic Meyer
//
//  This file is part of ARMigo.
//
//  ARMigo is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  ARMigo is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with ARMigo. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////

#include "../arm.hpp"
#define ARMIGO_INCLUDE

namespace armigo
{
    #include "arm_lut.hpp"

    template <bool immediate, int opcode, bool _set_flags, int field4>
    void arm::arm_data_processing(u32 instruction)
    {
        u32 op1, op2;
        bool carry = m_cpsr & MASK_CFLAG;
        int reg_dst = (instruction >> 12) & 0xF;
        int reg_op1 = (instruction >> 16) & 0xF;
        bool set_flags = _set_flags;

        op1 = m_reg[reg_op1];

        if (immediate)
        {
            int imm = instruction & 0xFF;
            int amount = ((instruction >> 8) & 0xF) << 1;

            if (amount != 0)
            {
                carry = (imm >> (amount -1)) & 1;
                op2 = (imm >> amount) | (imm << (32 - amount));
            }
            else
            {
                op2 = imm;
            }
        }
        else
        {
            u32 amount;
            int reg_op2 = instruction & 0xF;
            int shift_type = (field4 >> 1) & 3;
            bool shift_immediate = (field4 & 1) ? false : true;

            op2 = m_reg[reg_op2];

            if (!shift_immediate)
            {
                amount = m_reg[(instruction >> 8) & 0xF];
                if (reg_op1 == 15) op1 += 4;
                if (reg_op2 == 15) op2 += 4;
                cycles++;
            }
            else
            {
                amount = (instruction >> 7) & 0x1F;
            }

            perform_shift(shift_type, op2, amount, carry, shift_immediate);
        }

        if (reg_dst == 15)
        {
            if (set_flags)
            {
                u32 spsr = *m_spsr_ptr;
                switch_mode(static_cast<cpu_mode>(spsr & MASK_MODE));
                m_cpsr = spsr;
                set_flags = false;
            }
            m_flush = true;
        }

        switch (opcode)
        {
        case 0b0000:
        {
            // Bitwise AND (AND)
            u32 result = op1 & op2;

            if (set_flags)
            {
                update_sign(result);
                update_zero(result);
                set_carry(carry);
            }

            m_reg[reg_dst] = result;
            break;
        }
        case 0b0001:
        {
            // Bitwise EXOR (EOR)
            u32 result = op1 ^ op2;

            if (set_flags)
            {
                update_sign(result);
                update_zero(result);
                set_carry(carry);
            }

            m_reg[reg_dst] = result;
            break;
        }
        case 0b0010:
        {
            // Subtraction (SUB)
            u32 result = op1 - op2;

            if (set_flags)
            {
                set_carry(op1 >= op2);
                update_overflow_sub(result, op1, op2);
                update_sign(result);
                update_zero(result);
            }

            m_reg[reg_dst] = result;
            break;
        }
        case 0b0011:
        {
            // Reverse subtraction (RSB)
            u32 result = op2 - op1;

            if (set_flags)
            {
                set_carry(op2 >= op1);
                update_overflow_sub(result, op2, op1);
                update_sign(result);
                update_zero(result);
            }

            m_reg[reg_dst] = result;
            break;
        }
        case 0b0100:
        {
            // Addition (ADD)
            u32 result = op1 + op2;

            if (set_flags)
            {
                u64 result_long = (u64)op1 + (u64)op2;

                set_carry(result_long & 0x100000000);
                update_overflow_add(result, op1, op2);
                update_sign(result);
                update_zero(result);
            }

            m_reg[reg_dst] = result;
            break;
        }
        case 0b0101:
        {
            // Addition with Carry
            int carry2 = (m_cpsr >> 29) & 1;
            u32 result = op1 + op2 + carry2;

            if (set_flags)
            {
                u64 result_long = (u64)op1 + (u64)op2 + (u64)carry2;

                set_carry(result_long & 0x100000000);
                update_overflow_add(result, op1, op2);
                update_sign(result);
                update_zero(result);
            }

            m_reg[reg_dst] = result;
            break;
        }
        case 0b0110:
        {
            // Subtraction with Carry
            int carry2 = (m_cpsr >> 29) & 1;
            u32 result = op1 - op2 + carry2 - 1;

            if (set_flags)
            {
                set_carry(op1 >= (op2 + carry2 - 1));
                update_overflow_sub(result, op1, op2);
                update_sign(result);
                update_zero(result);
            }

            m_reg[reg_dst] = result;
            break;
        }
        case 0b0111:
        {
            // Reverse Substraction with Carry
            int carry2 = (m_cpsr >> 29) & 1;
            u32 result = op2 - op1 + carry2 - 1;

            if (set_flags)
            {
                set_carry(op2 >= (op1 + carry2 - 1));
                update_overflow_sub(result, op2, op1);
                update_sign(result);
                update_zero(result);
            }

            m_reg[reg_dst] = result;
            break;
        }
        case 0b1000:
        {
            // Bitwise AND flags only (TST)
            u32 result = op1 & op2;

            update_sign(result);
            update_zero(result);
            set_carry(carry);
            break;
        }
        case 0b1001:
        {
            // Bitwise EXOR flags only (TEQ)
            u32 result = op1 ^ op2;

            update_sign(result);
            update_zero(result);
            set_carry(carry);
            break;
        }
        case 0b1010:
        {
            // Subtraction flags only (CMP)
            u32 result = op1 - op2;

            set_carry(op1 >= op2);
            update_overflow_sub(result, op1, op2);
            update_sign(result);
            update_zero(result);
            break;
        }
        case 0b1011:
        {
            // Addition flags only (CMN)
            u32 result = op1 + op2;
            u64 result_long = (u64)op1 + (u64)op2;

            set_carry(result_long & 0x100000000);
            update_overflow_add(result, op1, op2);
            update_sign(result);
            update_zero(result);
            break;
        }
        case 0b1100:
        {
            // Bitwise OR (ORR)
            u32 result = op1 | op2;

            if (set_flags)
            {
                update_sign(result);
                update_zero(result);
                set_carry(carry);
            }

            m_reg[reg_dst] = result;
            break;
        }
        case 0b1101:
        {
            // Move into register (MOV)
            if (set_flags)
            {
                update_sign(op2);
                update_zero(op2);
                set_carry(carry);
            }

            m_reg[reg_dst] = op2;
            break;
        }
        case 0b1110:
        {
            // Bit Clear (BIC)
            u32 result = op1 & ~op2;

            if (set_flags)
            {
                update_sign(result);
                update_zero(result);
                set_carry(carry);
            }

            m_reg[reg_dst] = result;
            break;
        }
        case 0b1111:
        {
            // Move into register negated (MVN)
            u32 not_op2 = ~op2;

            if (set_flags)
            {
                update_sign(not_op2);
                update_zero(not_op2);
                set_carry(carry);
            }

            m_reg[reg_dst] = not_op2;
            break;
        }
        }
    }

    template <bool immediate, bool use_spsr, bool to_status>
    void arm::arm_psr_transfer(u32 instruction)
    {
        if (to_status)
        {
            u32 op;
            u32 mask = 0;

            // create mask based on fsxc-bits.
            if (instruction & (1 << 16)) mask |= 0x000000FF;
            if (instruction & (1 << 17)) mask |= 0x0000FF00;
            if (instruction & (1 << 18)) mask |= 0x00FF0000;
            if (instruction & (1 << 19)) mask |= 0xFF000000;

            // decode source operand
            if (immediate)
            {
                int imm = instruction & 0xFF;
                int amount = ((instruction >> 8) & 0xF) << 1;
                op = (imm >> amount) | (imm << (32 - amount));
            }
            else
            {
                op = m_reg[instruction & 0xF];
            }

            u32 value = op & mask;

            // write to cpsr or spsr
            if (!use_spsr)
            {
                // todo: check that mode is affected?
                switch_mode(static_cast<cpu_mode>(value & MASK_MODE));
                m_cpsr = (m_cpsr & ~mask) | value;
            }
            else
            {
                *m_spsr_ptr = (*m_spsr_ptr & ~mask) | value;
            }
        }
        else
        {
            int dst = (instruction >> 12) & 0xF;
            m_reg[dst] = use_spsr ? *m_spsr_ptr : m_cpsr;
        }
    }

    template <bool accumulate, bool set_flags>
    void arm::arm_multiply(u32 instruction)
    {
        u32 result;
        int op1 = instruction & 0xF;
        int op2 = (instruction >> 8) & 0xF;
        int dst = (instruction >> 16) & 0xF;

        result = m_reg[op1] * m_reg[op2];

        if (accumulate)
        {
            int op3 = (instruction >> 12) & 0xF;
            result += m_reg[op3];
        }

        if (set_flags)
        {
            update_sign(result);
            update_zero(result);
        }

        m_reg[dst] = result;
    }

    template <bool sign_extend, bool accumulate, bool set_flags>
    void arm::arm_multiply_long(u32 instruction)
    {
        s64 result;
        int op1 = instruction & 0xF;
        int op2 = (instruction >> 8) & 0xF;
        int dst_lo = (instruction >> 12) & 0xF;
        int dst_hi = (instruction >> 16) & 0xF;

        if (sign_extend)
        {
            s64 op1_value = m_reg[op1];
            s64 op2_value = m_reg[op2];

            // sign-extend operands
            if (op1_value & 0x80000000) op1_value |= 0xFFFFFFFF00000000;
            if (op2_value & 0x80000000) op2_value |= 0xFFFFFFFF00000000;

            result = op1_value * op2_value;
        }
        else
        {
            u64 uresult = (u64)m_reg[op1] * (u64)m_reg[op2];
            result = uresult;
        }

        if (accumulate)
        {
            s64 value = m_reg[dst_hi];

            // workaround required by x86.
            value <<= 16;
            value <<= 16;
            value |= m_reg[dst_lo];

            result += value;
        }

        u32 result_hi = result >> 32;

        m_reg[dst_lo] = result & 0xFFFFFFFF;
        m_reg[dst_hi] = result_hi;

        if (set_flags)
        {
            update_sign(result_hi);
            update_zero(result);
        }
    }

    template <bool swap_byte>
    void arm::arm_single_data_swap(u32 instruction)
    {
        u32 tmp;
        int src = instruction & 0xF;
        int dst = (instruction >> 12) & 0xF;
        int base = (instruction >> 16) & 0xF;

        if (swap_byte)
        {
            tmp = bus_read_byte(m_reg[base]);
            bus_write_byte(m_reg[base], (u8)m_reg[src]);
        }
        else
        {
            tmp = read_word_rotated(m_reg[base]);
            write_word(m_reg[base], m_reg[src]);
        }

        m_reg[dst] = tmp;
    }

    void arm::arm_branch_exchange(u32 instruction)
    {
        u32 addr = m_reg[instruction & 0xF];

        m_flush = true;

        if (addr & 1)
        {
            m_reg[15] = addr & ~1;
            m_cpsr |= MASK_THUMB;
        }
        else
        {
            m_reg[15] = addr & ~3;
        }
    }

    template <bool pre_indexed, bool base_increment, bool immediate, bool write_back, bool load, int opcode>
    void arm::arm_halfword_signed_transfer(u32 instruction)
    {
        u32 off;
        int dst = (instruction >> 12) & 0xF;
        int base = (instruction >> 16) & 0xF;
        u32 addr = m_reg[base];

        if (immediate)
            off = (instruction & 0xF) | ((instruction >> 4) & 0xF0);
        else
            off = m_reg[instruction & 0xF];

        if (pre_indexed)
            addr += base_increment ? off : -off;

        switch (opcode)
        {
        case 1:
            // load/store halfword
            if (load)
            {
                m_reg[dst] = read_hword(addr);
            }
            else
            {
                u32 value = m_reg[dst];

                // r15 is $+12 instead of $+8 because of internal prefetch.
                if (dst == 15) value += 4;

                write_hword(addr, value);
            }
            break;
        case 2:
        {
            // load signed byte
            u32 value = bus_read_byte(addr);

            // sign-extends the byte to 32-bit.
            if (value & 0x80) value |= 0xFFFFFF00;

            m_reg[dst] = value;
            break;
        }
        case 3:
        {
            // load signed halfword
            u32 value = read_hword(addr);

            // sign-extends the halfword to 32-bit.
            if (value & 0x8000) value |= 0xFFFF0000;

            m_reg[dst] = value;
            break;
        }
        }

        if ((write_back || !pre_indexed) && base != dst)
        {
            if (!pre_indexed)
                addr += base_increment ? off : -off;
            m_reg[base] = addr;
        }
    }

    template <bool immediate, bool pre_indexed, bool base_increment, bool byte, bool write_back, bool load>
    void arm::arm_single_transfer(u32 instruction)
    {
        u32 off;
        cpu_mode old_mode;
        int dst = (instruction >> 12) & 0xF;
        int base = (instruction >> 16) & 0xF;
        u32 addr = m_reg[base];

        // post-indexing implicitly performs a write back.
        // in that case W-bit indicates wether user-mode register access should be enforced.
        if (!pre_indexed && write_back)
        {
            old_mode = static_cast<cpu_mode>(m_cpsr & MASK_MODE);
            switch_mode(MODE_USR);
        }

        // get address offset.
        if (immediate)
        {
            off = instruction & 0xFFF;
        }
        else
        {
            bool carry = m_cpsr & MASK_CFLAG;
            int shift = (instruction >> 5) & 3;
            u32 amount = (instruction >> 7) & 0x1F;

            off = m_reg[instruction & 0xF];
            perform_shift(shift, off, amount, carry, true);
        }

        if (pre_indexed) addr += base_increment ? off : -off;

        if (load)
        {
            m_reg[dst] = byte ? bus_read_byte(addr) : read_word_rotated(addr);

            // writes to r15 require a pipeline flush.
            if (dst == 15) m_flush = true;
        }
        else
        {
            u32 value = m_reg[dst];

            // r15 is $+12 now due to internal prefetch cycle.
            if (dst == 15) value += 4;

            byte ? bus_write_byte(addr, (u8)value) : write_word(addr, value);
        }

        // writeback operation may not overwrite the destination register.
        if (base != dst)
        {
            if (!pre_indexed)
            {
                m_reg[base] += base_increment ? off : -off;

                // if user-mode was enforced, return to previous mode.
                if (write_back) switch_mode(old_mode);
            }
            else if (write_back)
            {
                m_reg[base] = addr;
            }
        }
    }

    void arm::arm_undefined(u32 instruction)
    {
        // save return address and program status
        m_bank[BANK_SVC][BANK_R14] = m_reg[15] - 4;
        m_spsr[SPSR_SVC] = m_cpsr;

        // switch to UND mode and disable interrupts
        switch_mode(MODE_UND);
        m_cpsr |= MASK_IRQD;

        // jump to exception vector
        m_reg[15] = EXCPT_UNDEFINED;
        m_flush = true;
    }

    template <bool _pre_indexed, bool base_increment, bool user_mode, bool _write_back, bool load>
    void arm::arm_block_transfer(u32 instruction)
    {
        int first_register;
        int register_count = 0;
        int register_list = instruction & 0xFFFF;
        bool transfer_r15 = register_list & (1 << 15);
        bool pre_indexed = _pre_indexed;
        bool write_back = _write_back;

        int base = (instruction >> 16) & 0xF;

        cpu_mode old_mode;
        bool mode_switched = false;

        // hardware corner case. not sure if emulated correctly.
        if (register_list == 0)
        {
            if (load)
            {
                m_reg[15] = read_word(m_reg[base]);
                m_flush = true;
            }
            else
            {
                write_word(m_reg[base], m_reg[15]);
            }
            m_reg[base] += base_increment ? 64 : -64;
            return;
        }

        if (user_mode && (!load || !transfer_r15))
        {
            old_mode = static_cast<cpu_mode>(m_cpsr & MASK_MODE);
            switch_mode(MODE_USR);
            mode_switched = true;
        }

        // find first register in list and also count them.
        for (int i = 15; i >= 0; i--)
        {
            if (~register_list & (1 << i)) continue;

            first_register = i;
            register_count++;
        }

        u32 addr = m_reg[base];
        u32 addr_old = addr;

        // instruction internally works with incrementing addresses.
        if (!base_increment)
        {
            addr -= register_count * 4;
            m_reg[base] = addr;
            write_back = false;
            pre_indexed = !pre_indexed;
        }

        // process register list starting with the lowest address and register.
        for (int i = first_register; i < 16; i++)
        {
            if (~register_list & (1 << i)) continue;

            if (pre_indexed) addr += 4;

            if (load)
            {
                if (i == base) write_back = false;

                m_reg[i] = read_word(addr);

                if (i == 15)
                {
                    if (user_mode)
                    {
                        u32 spsr = *m_spsr_ptr;
                        switch_mode(static_cast<cpu_mode>(spsr & MASK_MODE));
                        m_cpsr = spsr;
                    }
                    m_flush = true;
                }
            }
            else
            {
                if (i == first_register && i == base)
                    write_word(addr, addr_old);
                else
                    write_word(addr, m_reg[i]);
            }

            if (!pre_indexed) addr += 4;

            if (write_back) m_reg[base] = addr;
        }

        if (mode_switched) switch_mode(old_mode);
    }

    template <bool link>
    void arm::arm_branch(u32 instruction)
    {
        u32 off = instruction & 0xFFFFFF;

        if (off & 0x800000) off |= 0xFF000000;
        if (link) m_reg[14] = m_reg[15] - 4;

        m_reg[15] += off << 2;
        m_flush = true;
    }

    void arm::arm_swi(u32 instruction)
    {
        u32 call_number = bus_read_byte(m_reg[15] - 6);

        if (!m_hle)
        {
            // save return address and program status
            m_bank[BANK_SVC][BANK_R14] = m_reg[15] - 4;
            m_spsr[SPSR_SVC] = m_cpsr;

            // switch to SVC mode and disable interrupts
            switch_mode(MODE_SVC);
            m_cpsr |= MASK_IRQD;

            // jump to exception vector
            m_reg[15] = EXCPT_SWI;
            m_flush = true;
        }
        else
        {
            software_interrupt(call_number);
        }
    }
}
