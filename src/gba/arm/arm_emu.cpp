///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2016 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////

#include "arm.hpp"

const int ARM_1 = 1;
const int ARM_2 = 2;
const int ARM_3 = 3;
const int ARM_4 = 4;
const int ARM_5 = 5;
const int ARM_6 = 6;
const int ARM_7 = 7;
const int ARM_8 = 8;
const int ARM_9 = 9;
const int ARM_10 = 10;
const int ARM_11 = 11;
const int ARM_12 = 12;
const int ARM_13 = 13;
const int ARM_14 = 14;
const int ARM_15 = 15;
const int ARM_16 = 16;

namespace GBA
{
    int arm::arm_decode(u32 instruction)
    {
        u32 opcode = instruction & 0x0FFFFFFF;

        switch (opcode >> 26)
        {
        case 0b00:
            if (opcode & (1 << 25))
            {
                // ARM.8 Data processing and PSR transfer ... immediate
                return ARM_8;
            }
            else if ((opcode & 0xFF00FF0) == 0x1200F10)
            {
                // ARM.3 Branch and exchange
                return ARM_3;
            }
            else if ((opcode & 0x10000F0) == 0x90)
            {
                // ARM.1 Multiply (accumulate), ARM.2 Multiply (accumulate) long
                return opcode & (1 << 23) ? ARM_2 : ARM_1;
            }
            else if ((opcode & 0x10000F0) == 0x1000090)
            {
                // ARM.4 Single data swap
                return ARM_4;
            }
            else if ((opcode & 0x4000F0) == 0xB0)
            {
                // ARM.5 Halfword data transfer, register offset
                return ARM_5;
            }
            else if ((opcode & 0x4000F0) == 0x4000B0)
            {
                // ARM.6 Halfword data transfer, immediate offset
                return ARM_6;
            }
            else if ((opcode & 0xD0) == 0xD0)
            {
                // ARM.7 Signed data transfer (byte/halfword)
                return ARM_7;
            }
            else
            {
                // ARM.8 Data processing and PSR transfer
                return ARM_8;
            }
            break;
        case 0b01:
            // ARM.9 Single data transfer, ARM.10 Undefined
            return (opcode & 0x2000010) == 0x2000010 ? ARM_10 : ARM_9;
        case 0b10:
            // ARM.11 Block data transfer, ARM.12 Branch
            return opcode & (1 << 25) ? ARM_12 : ARM_11;
        case 0b11:
            if (opcode & (1 << 25))
            {
                if (opcode & (1 << 24))
                {
                    // ARM.16 Software interrupt
                    return ARM_16;
                }
                else
                {
                    // ARM.14 Coprocessor data operation, ARM.15 Coprocessor register transfer
                    return opcode & 0x10 ? ARM_15 : ARM_14;
                }
            }
            else
            {
                // ARM.13 Coprocessor data transfer
                return ARM_13;
            }
            break;
        }

        return 0;
    }

    void arm::arm_multiply(u32 instruction, bool accumulate, bool set_flags)
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

    void arm::arm_multiply_long(u32 instruction, bool sign_extend, bool accumulate, bool set_flags)
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

    void arm::arm_single_data_swap(u32 instruction, bool swap_byte)
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

        m_pipeline.m_needs_flush = true;

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

    void arm::arm_halfword_signed_transfer(u32 instruction, bool pre_indexed, bool base_increment, bool immediate, bool write_back, bool load, int opcode)
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

    void arm::arm_single_transfer(u32 instruction, bool immediate, bool pre_indexed, bool base_increment, bool byte, bool write_back, bool load)
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
            if (dst == 15) m_pipeline.m_needs_flush = true;
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
        // todo
    }

    void arm::arm_block_transfer(u32 instruction, bool pre_indexed, bool base_increment, bool user_mode, bool write_back, bool load)
    {
        int first_register;
        int register_count = 0;
        int register_list = instruction & 0xFFFF;
        bool transfer_r15 = register_list & (1 << 15);

        int base = (instruction >> 16) & 0xF;

        cpu_mode old_mode;
        bool mode_switched = false;

        // hardware corner case. not sure if emulated correctly.
        if (register_list == 0)
        {
            if (load)
                m_reg[15] = read_word(m_reg[base]);
            else
                write_word(m_reg[base], m_reg[15]);
            m_reg[base] += base_increment ? 64 : -64;
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
                    m_pipeline.m_needs_flush = true;
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

    void arm::arm_branch(u32 instruction, bool link)
    {
        u32 off = instruction & 0xFFFFFF;

        if (off & 0x800000) off |= 0xFF000000;
        if (link) m_reg[14] = m_reg[15] - 4;

        m_reg[15] += off << 2;
        m_pipeline.m_needs_flush = true;
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

            // jump to execution vector
            m_reg[15] = EXCPT_SWI;
            m_pipeline.m_needs_flush = true;
        }
        else
        {
            software_interrupt(call_number);
        }
    }

    void arm::arm_execute(u32 instruction, int type)
    {
        auto reg = m_reg;
        int condition = instruction >> 28;

        // Check if the instruction will be executed
        if (condition != 0xE)
        {
            switch (condition)
            {
            case 0x0: if (!(m_cpsr & MASK_ZFLAG))     return; break;
            case 0x1: if (  m_cpsr & MASK_ZFLAG)      return; break;
            case 0x2: if (!(m_cpsr & MASK_CFLAG))    return; break;
            case 0x3: if (  m_cpsr & MASK_CFLAG)     return; break;
            case 0x4: if (!(m_cpsr & MASK_NFLAG))     return; break;
            case 0x5: if (  m_cpsr & MASK_NFLAG)      return; break;
            case 0x6: if (!(m_cpsr & MASK_VFLAG)) return; break;
            case 0x7: if (  m_cpsr & MASK_VFLAG)  return; break;
            case 0x8: if (!(m_cpsr & MASK_CFLAG) ||  (m_cpsr & MASK_ZFLAG)) return; break;
            case 0x9: if ( (m_cpsr & MASK_CFLAG) && !(m_cpsr & MASK_ZFLAG)) return; break;
            case 0xA: if ((m_cpsr & MASK_NFLAG) != (m_cpsr & MASK_VFLAG)) return; break;
            case 0xB: if ((m_cpsr & MASK_NFLAG) == (m_cpsr & MASK_VFLAG)) return; break;
            case 0xC: if ((m_cpsr & MASK_ZFLAG) || ((m_cpsr & MASK_NFLAG) != (m_cpsr & MASK_VFLAG))) return; break;
            case 0xD: if (!(m_cpsr & MASK_ZFLAG) && ((m_cpsr & MASK_NFLAG) == (m_cpsr & MASK_VFLAG))) return; break;
            case 0xE: break;
            case 0xF: return;
            }
        }

        // Perform the actual execution
        switch (type)
        {
        case ARM_1:
        {
            bool accumulate = instruction & (1 << 21);
            bool set_flags = instruction & (1 << 20);
            arm_multiply(instruction, accumulate, set_flags);
            return;
        }
        case ARM_2:
        {
            bool sign_extend = instruction & (1 << 22);
            bool accumulate = instruction & (1 << 21);
            bool set_flags = instruction & (1 << 20);
            arm_multiply_long(instruction, sign_extend, accumulate, set_flags);
            return;
        }
        case ARM_3:
        {
            arm_branch_exchange(instruction);
            return;
        }
        case ARM_4:
        {
            bool swap_byte = instruction & (1 << 22);
            arm_single_data_swap(instruction, swap_byte);
            return;
        }
        case ARM_5:
        case ARM_6:
        case ARM_7:
        {
            bool pre_indexed = instruction & (1 << 24);
            bool base_increment = instruction & (1 << 23);
            bool immediate = instruction & (1 << 22);
            bool write_back = instruction & (1 << 21);
            bool load = instruction & (1 << 20);
            int opcode = (instruction >> 5) & 3;

            arm_halfword_signed_transfer(instruction, pre_indexed, base_increment, immediate, write_back, load, opcode);
            return;
        }
        case ARM_8:
        {
            // ARM.8 Data processing and PSR transfer
            bool set_flags = instruction & (1 << 20);
            int opcode = (instruction >> 21) & 0xF;

            // Decide wether the instruction is data processing or psr transfer
            // Look at official ARM docs to understand this condition!
            if (!set_flags && opcode >= 0b1000 && opcode <= 0b1011)
            {
                // PSR transfer
                bool immediate = instruction & (1 << 25);
                bool use_spsr = instruction & (1 << 22);
                bool to_status = instruction & (1 << 21);

                if (to_status)
                {
                    // Move register to status register.
                    u32 mask = 0;
                    u32 operand;

                    // Depending of the fsxc bits some bits are overwritten or not
                    if (instruction & (1 << 16)) mask |= 0x000000FF;
                    if (instruction & (1 << 17)) mask |= 0x0000FF00;
                    if (instruction & (1 << 18)) mask |= 0x00FF0000;
                    if (instruction & (1 << 19)) mask |= 0xFF000000;

                    // Decode the value written to m_cpsr/spsr
                    if (immediate)
                    {
                        int imm = instruction & 0xFF;
                        int ror = ((instruction >> 8) & 0xF) << 1;

                        // Apply immediate rotate_right-shift.
                        operand = (imm >> ror) | (imm << (32 - ror));
                    }
                    else
                    {
                        operand = reg[instruction & 0xF];
                    }

                    // Finally write either to SPSR or m_cpsr.
                    if (use_spsr)
                    {
                        *m_spsr_ptr = (*m_spsr_ptr & ~mask) | (operand & mask);
                    }
                    else
                    {
                        ////SaveRegisters();
                        ////m_cpsr = (m_cpsr & ~mask) | (operand & mask);
                        ////LoadRegisters();
                        u32 value = operand & mask;

                        // todo: make sure that mode might actually be affected.
                        switch_mode(static_cast<cpu_mode>(value & MASK_MODE));
                        m_cpsr = (m_cpsr & ~mask) | value;
                    }
                }
                else
                {
                    // Move satus register to register.
                    int reg_dest = (instruction >> 12) & 0xF;
                    reg[reg_dest] = use_spsr ? *m_spsr_ptr : m_cpsr;
                }
            }
            else
            {
                // Data processing
                int reg_dest = (instruction >> 12) & 0xF;
                int reg_operand1 = (instruction >> 16) & 0xF;
                bool immediate = instruction & (1 << 25);
                bool carry = m_cpsr & MASK_CFLAG;
                u32 operand1 = reg[reg_operand1];
                u32 operand2;

                // Operand 2 may be an immediate value or a shifted register.
                if (immediate)
                {
                    int immediate_value = instruction & 0xFF;
                    int amount = ((instruction >> 8) & 0xF) << 1;

                    // Apply hardcoded rotation
                    operand2 = (immediate_value >> amount) | (immediate_value << (32 - amount));

                    // Rotation also overwrites the carry flag
                    if (amount != 0)
                        carry = (immediate_value >> (amount - 1)) & 1;
                }
                else
                {
                    u32 amount;
                    int reg_operand2 = instruction & 0xF;
                    bool shift_immediate = (instruction & (1 << 4)) ? false : true;

                    // Get operand raw value
                    operand2 = reg[reg_operand2];

                    // Get amount of shift.
                    if (shift_immediate)
                    {
                        amount = (instruction >> 7) & 0x1F;
                    }
                    else
                    {
                        int reg_shift = (instruction >> 8) & 0xF;
                        amount = reg[reg_shift];

                        // Internal cycle
                        cycles++;

                        // Due to the internal shift cycle r15 will be 12 bytes ahead, not 8.
                        if (reg_operand1 == 15) operand1 += 4;
                        if (reg_operand2 == 15) operand2 += 4;
                    }

                    // Apply shift by "amount" bits to raw value.
                    switch ((instruction >> 5) & 3)
                    {
                    case 0b00:
                        logical_shift_left(operand2, amount, carry);
                        break;
                    case 0b01:
                        logical_shift_right(operand2, amount, carry, shift_immediate);
                        break;
                    case 0b10:
                        arithmetic_shift_right(operand2, amount, carry, shift_immediate);
                        break;
                    case 0b11:
                        rotate_right(operand2, amount, carry, shift_immediate);
                        break;
                    }
                }

                // The combination rDEST=15 and S-bit=1 triggers a CPU mode switch
                // effectivly switchting back to the saved mode (m_cpsr = SPSR).
                if (reg_dest == 15 && set_flags)
                {
                    // Flags will no longer be updated.
                    set_flags = false;

                    // Switches back to saved mode.
                    ////SaveRegisters();
                    ////m_cpsr = *m_spsr_ptr;
                    ////LoadRegisters();
                    u32 spsr = *m_spsr_ptr;
                    switch_mode(static_cast<cpu_mode>(spsr & MASK_MODE));
                    m_cpsr = spsr;
                }

                // Perform the actual operation
                switch (opcode)
                {
                case 0b0000:
                {
                    // Bitwise AND (AND)
                    u32 result = operand1 & operand2;

                    if (set_flags)
                    {
                        update_sign(result);
                        update_zero(result);
                        set_carry(carry);
                    }

                    reg[reg_dest] = result;
                    break;
                }
                case 0b0001:
                {
                    // Bitwise EXOR (EOR)
                    u32 result = operand1 ^ operand2;

                    if (set_flags)
                    {
                        update_sign(result);
                        update_zero(result);
                        set_carry(carry);
                    }

                    reg[reg_dest] = result;
                    break;
                }
                case 0b0010:
                {
                    // Subtraction (SUB)
                    u32 result = operand1 - operand2;

                    if (set_flags)
                    {
                        set_carry(operand1 >= operand2);
                        update_overflow_sub(result, operand1, operand2);
                        update_sign(result);
                        update_zero(result);
                    }

                    reg[reg_dest] = result;
                    break;
                }
                case 0b0011:
                {
                    // Reverse subtraction (RSB)
                    u32 result = operand2 - operand1;

                    if (set_flags)
                    {
                        set_carry(operand2 >= operand1);
                        update_overflow_sub(result, operand2, operand1);
                        update_sign(result);
                        update_zero(result);
                    }

                    reg[reg_dest] = result;
                    break;
                }
                case 0b0100:
                {
                    // Addition (ADD)
                    u32 result = operand1 + operand2;

                    if (set_flags)
                    {
                        u64 result_long = (u64)operand1 + (u64)operand2;

                        set_carry(result_long & 0x100000000);
                        update_overflow_add(result, operand1, operand2);
                        update_sign(result);
                        update_zero(result);
                    }

                    reg[reg_dest] = result;
                    break;
                }
                case 0b0101:
                {
                    // Addition with Carry
                    int carry2 = (m_cpsr >> 29) & 1;
                    u32 result = operand1 + operand2 + carry2;

                    if (set_flags)
                    {
                        u64 result_long = (u64)operand1 + (u64)operand2 + (u64)carry2;

                        set_carry(result_long & 0x100000000);
                        update_overflow_add(result, operand1, operand2);
                        update_sign(result);
                        update_zero(result);
                    }

                    reg[reg_dest] = result;
                    break;
                }
                case 0b0110:
                {
                    // Subtraction with Carry
                    int carry2 = (m_cpsr >> 29) & 1;
                    u32 result = operand1 - operand2 + carry2 - 1;

                    if (set_flags)
                    {
                        set_carry(operand1 >= (operand2 + carry2 - 1));
                        update_overflow_sub(result, operand1, operand2);
                        update_sign(result);
                        update_zero(result);
                    }

                    reg[reg_dest] = result;
                    break;
                }
                case 0b0111:
                {
                    // Reverse Substraction with Carry
                    int carry2 = (m_cpsr >> 29) & 1;
                    u32 result = operand2 - operand1 + carry2 - 1;

                    if (set_flags)
                    {
                        set_carry(operand2 >= (operand1 + carry2 - 1));
                        update_overflow_sub(result, operand2, operand1);
                        update_sign(result);
                        update_zero(result);
                    }

                    reg[reg_dest] = result;
                    break;
                }
                case 0b1000:
                {
                    // Bitwise AND flags only (TST)
                    u32 result = operand1 & operand2;

                    update_sign(result);
                    update_zero(result);
                    set_carry(carry);
                    break;
                }
                case 0b1001:
                {
                    // Bitwise EXOR flags only (TEQ)
                    u32 result = operand1 ^ operand2;

                    update_sign(result);
                    update_zero(result);
                    set_carry(carry);
                    break;
                }
                case 0b1010:
                {
                    // Subtraction flags only (CMP)
                    u32 result = operand1 - operand2;

                    set_carry(operand1 >= operand2);
                    update_overflow_sub(result, operand1, operand2);
                    update_sign(result);
                    update_zero(result);
                    break;
                }
                case 0b1011:
                {
                    // Addition flags only (CMN)
                    u32 result = operand1 + operand2;
                    u64 result_long = (u64)operand1 + (u64)operand2;

                    set_carry(result_long & 0x100000000);
                    update_overflow_add(result, operand1, operand2);
                    update_sign(result);
                    update_zero(result);
                    break;
                }
                case 0b1100:
                {
                    // Bitwise OR (ORR)
                    u32 result = operand1 | operand2;

                    if (set_flags)
                    {
                        update_sign(result);
                        update_zero(result);
                        set_carry(carry);
                    }

                    reg[reg_dest] = result;
                    break;
                }
                case 0b1101:
                {
                    // Move into register (MOV)
                    if (set_flags)
                    {
                        update_sign(operand2);
                        update_zero(operand2);
                        set_carry(carry);
                    }

                    reg[reg_dest] = operand2;
                    break;
                }
                case 0b1110:
                {
                    // Bit Clear (BIC)
                    u32 result = operand1 & ~operand2;

                    if (set_flags)
                    {
                        update_sign(result);
                        update_zero(result);
                        set_carry(carry);
                    }

                    reg[reg_dest] = result;
                    break;
                }
                case 0b1111:
                {
                    // Move into register negated (MVN)
                    u32 not_operand2 = ~operand2;

                    if (set_flags)
                    {
                        update_sign(not_operand2);
                        update_zero(not_operand2);
                        set_carry(carry);
                    }

                    reg[reg_dest] = not_operand2;
                    break;
                }
                }

                // Clear pipeline if r15 updated
                if (reg_dest == 15)
                {
                    m_pipeline.m_needs_flush = true;
                }
            }
            return;
        }
        case ARM_9:
        {
            bool immediate = (instruction & (1 << 25)) == 0;
            bool pre_indexed = instruction & (1 << 24);
            bool add_to_base = instruction & (1 << 23);
            bool transfer_byte = instruction & (1 << 22);
            bool write_back = instruction & (1 << 21);
            bool load = instruction & (1 << 20);

            arm_single_transfer(instruction, immediate, pre_indexed, add_to_base, transfer_byte, write_back, load);
            return;
        }
        case ARM_10:
            // ARM.10 Undefined
            #ifdef DEBUG
            LOG(LOG_ERROR, "Undefined instruction (0x%x), r15=0x%x", instruction, reg[15]);
            #endif
            return;
        case ARM_11:
        {
            bool pre_indexed = instruction & (1 << 24);
            bool increment_base = instruction & (1 << 23);
            bool force_user_mode = instruction & (1 << 22);
            bool write_back = instruction & (1 << 21);
            bool load_instr = instruction & (1 << 20);
            arm_block_transfer(instruction, pre_indexed, increment_base, force_user_mode, write_back, load_instr);
            return;
        }
        case ARM_12:
        {
            bool link = instruction & (1 << 24);
            arm_branch(instruction, link);
            return;
        }
        case ARM_16:
        {
            arm_swi(instruction);
            return;
        }
        }
    }
}
