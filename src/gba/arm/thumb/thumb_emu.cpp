///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2017 Frederic Meyer
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

#include "../arm.hpp"
#define ARMIGO_INCLUDE

namespace armigo
{
    #include "thumb_lut.hpp"

    template <int imm, int type>
    void arm::thumb_1(u16 instruction)
    {
        // THUMB.1 Move shifted register
        int dst = instruction & 7;
        int src = (instruction >> 3) & 7;
        bool carry = m_cpsr & MASK_CFLAG;

        m_reg[dst] = m_reg[src];

        perform_shift(type, m_reg[dst], imm, carry, true);

        // update carry, sign and zero flag
        set_carry(carry);
        update_sign(m_reg[dst]);
        update_zero(m_reg[dst]);
    }

    template <bool immediate, bool subtract, int field3>
    void arm::thumb_2(u16 instruction)
    {
        // THUMB.2 Add/subtract
        u32 operand, result;
        int dst = instruction & 7;
        int src = (instruction >> 3) & 7;

        // either a register or an immediate value
        operand = immediate ? field3 : m_reg[field3];

        if (subtract)
        {
            result = m_reg[src] - operand;

            set_carry(m_reg[src] >= operand);
            update_overflow_sub(result, m_reg[src], operand);
        }
        else
        {
            u64 result_long = (u64)(m_reg[src]) + (u64)operand;

            result = (u32)result_long;
            set_carry(result_long & 0x100000000);
            update_overflow_add(result, m_reg[src], operand);
        }

        update_sign(result);
        update_zero(result);

        m_reg[dst] = result;
    }

    template <int op, int dst>
    void arm::thumb_3(u16 instruction)
    {
        // THUMB.3 Move/compare/add/subtract immediate
        u32 result;
        u32 immediate_value = instruction & 0xFF;

        switch (op)
        {
        case 0b00: // MOV
            m_reg[dst] = immediate_value;
            update_sign(0);
            update_zero(immediate_value);
            return;//important!

        case 0b01: // CMP
            result = m_reg[dst] - immediate_value;
            set_carry(m_reg[dst] >= immediate_value);
            update_overflow_sub(result, m_reg[dst], immediate_value);
            break;
        case 0b10: // ADD
        {
            u64 result_long = (u64)m_reg[dst] + (u64)immediate_value;
            result = (u32)result_long;
            set_carry(result_long & 0x100000000);
            update_overflow_add(result, m_reg[dst], immediate_value);
            m_reg[dst] = result;
            break;
        }
        case 0b11: // SUB
            result = m_reg[dst] - immediate_value;
            set_carry(m_reg[dst] >= immediate_value);
            update_overflow_sub(result, m_reg[dst], immediate_value);
            m_reg[dst] = result;
            break;
        }

        update_sign(result);
        update_zero(result);
    }

    template <int op>
    void arm::thumb_4(u16 instruction)
    {
        // THUMB.4 ALU operations
        int dst = instruction & 7;
        int src = (instruction >> 3) & 7;

        switch (op)
        {
        case 0b0000: // AND
            m_reg[dst] &= m_reg[src];
            update_sign(m_reg[dst]);
            update_zero(m_reg[dst]);
            break;
        case 0b0001: // EOR
            m_reg[dst] ^= m_reg[src];
            update_sign(m_reg[dst]);
            update_zero(m_reg[dst]);
            break;
        case 0b0010: // LSL
        {
            u32 amount = m_reg[src];
            bool carry = m_cpsr & MASK_CFLAG;
            logical_shift_left(m_reg[dst], amount, carry);
            set_carry(carry);
            update_sign(m_reg[dst]);
            update_zero(m_reg[dst]);
            cycles++;
            break;
        }
        case 0b0011: // LSR
        {
            u32 amount = m_reg[src];
            bool carry = m_cpsr & MASK_CFLAG;
            logical_shift_right(m_reg[dst], amount, carry, false);
            set_carry(carry);
            update_sign(m_reg[dst]);
            update_zero(m_reg[dst]);
            cycles++;
            break;
        }
        case 0b0100: // ASR
        {
            u32 amount = m_reg[src];
            bool carry = m_cpsr & MASK_CFLAG;
            arithmetic_shift_right(m_reg[dst], amount, carry, false);
            set_carry(carry);
            update_sign(m_reg[dst]);
            update_zero(m_reg[dst]);
            cycles++;
            break;
        }
        case 0b0101: // ADC
        {
            int carry = (m_cpsr >> 29) & 1;
            u32 result = m_reg[dst] + m_reg[src] + carry;
            u64 result_long = (u64)(m_reg[dst]) + (u64)(m_reg[src]) + (u64)carry;
            set_carry(result_long & 0x100000000);
            update_overflow_add(result, m_reg[dst], m_reg[src]);
            update_sign(result);
            update_zero(result);
            m_reg[dst] = result;
            break;
        }
        case 0b0110: // SBC
        {
            int carry = (m_cpsr >> 29) & 1;
            u32 result = m_reg[dst] - m_reg[src] + carry - 1;
            set_carry(m_reg[dst] >= (m_reg[src] + carry - 1));
            update_overflow_sub(result, m_reg[dst], m_reg[src]);
            update_sign(result);
            update_zero(result);
            m_reg[dst] = result;
            break;
        }
        case 0b0111: // ROR
        {
            u32 amount = m_reg[src];
            bool carry = m_cpsr & MASK_CFLAG;
            rotate_right(m_reg[dst], amount, carry, false);
            set_carry(carry);
            update_sign(m_reg[dst]);
            update_zero(m_reg[dst]);
            cycles++;
            break;
        }
        case 0b1000: // TST
        {
            u32 result = m_reg[dst] & m_reg[src];
            update_sign(result);
            update_zero(result);
            break;
        }
        case 0b1001: // NEG
        {
            u32 result = 0 - m_reg[src];
            set_carry(0 >= m_reg[src]);
            update_overflow_sub(result, 0, m_reg[src]);
            update_sign(result);
            update_zero(result);
            m_reg[dst] = result;
            break;
        }
        case 0b1010: // CMP
        {
            u32 result = m_reg[dst] - m_reg[src];
            set_carry(m_reg[dst] >= m_reg[src]);
            update_overflow_sub(result, m_reg[dst], m_reg[src]);
            update_sign(result);
            update_zero(result);
            break;
        }
        case 0b1011: // CMN
        {
            u32 result = m_reg[dst] + m_reg[src];
            u64 result_long = (u64)(m_reg[dst]) + (u64)(m_reg[src]);
            set_carry(result_long & 0x100000000);
            update_overflow_add(result, m_reg[dst], m_reg[src]);
            update_sign(result);
            update_zero(result);
            break;
        }
        case 0b1100: // ORR
            m_reg[dst] |= m_reg[src];
            update_sign(m_reg[dst]);
            update_zero(m_reg[dst]);
            break;
        case 0b1101: // MUL
            // TODO: how to calc. the internal cycles?
            m_reg[dst] *= m_reg[src];
            update_sign(m_reg[dst]);
            update_zero(m_reg[dst]);
            set_carry(false);
            break;
        case 0b1110: // BIC
            m_reg[dst] &= ~(m_reg[src]);
            update_sign(m_reg[dst]);
            update_zero(m_reg[dst]);
            break;
        case 0b1111: // MVN
            m_reg[dst] = ~(m_reg[src]);
            update_sign(m_reg[dst]);
            update_zero(m_reg[dst]);
            break;
        }
    }

    template <int op, bool high1, bool high2>
    void arm::thumb_5(u16 instruction)
    {
        // THUMB.5 Hi register operations/branch exchange
        u32 operand;
        bool perform_check = true;
        int dst = instruction & 7;
        int src = (instruction >> 3) & 7;

        if (high1) dst |= 8;
        if (high2) src |= 8;

        operand = m_reg[src];
        if (src == 15) operand &= ~1;

        switch (op)
        {
        case 0: // ADD
            m_reg[dst] += operand;
            break;
        case 1: // CMP
        {
            u32 result = m_reg[dst] - operand;
            set_carry(m_reg[dst] >= operand);
            update_overflow_sub(result, m_reg[dst], operand);
            update_sign(result);
            update_zero(result);
            perform_check = false;
            break;
        }
        case 2: // MOV
            m_reg[dst] = operand;
            break;
        case 3: // BX
            if (operand & 1)
            {
                m_reg[15] = operand & ~1;
            }
            else
            {
                m_cpsr &= ~MASK_THUMB;
                m_reg[15] = operand & ~3;
            }
            m_pipeline.m_needs_flush = true;
            perform_check = false;
            break;
        }

        if (perform_check && dst == 15)
        {
            m_reg[dst] &= ~1;
            m_pipeline.m_needs_flush = true;
        }
    }

    template <int dst>
    void arm::thumb_6(u16 instruction)
    {
        // THUMB.6 PC-relative load
        u32 immediate_value = instruction & 0xFF;
        u32 address = (m_reg[15] & ~2) + (immediate_value << 2);

        m_reg[dst] = read_word(address);
    }

    template <int op, int off>
    void arm::thumb_7(u16 instruction)
    {
        // THUMB.7 Load/store with register offset
        int dst = instruction & 7;
        int base = (instruction >> 3) & 7;
        u32 address = m_reg[base] + m_reg[off];

        switch (op)
        {
        case 0b00: // STR
            write_word(address, m_reg[dst]);
            break;
        case 0b01: // STRB
            bus_write_byte(address, (u8)m_reg[dst]);
            break;
        case 0b10: // LDR
            m_reg[dst] = read_word_rotated(address);
            break;
        case 0b11: // LDRB
            m_reg[dst] = bus_read_byte(address);
            break;
        }
    }

    template <int op, int off>
    void arm::thumb_8(u16 instruction)
    {
        // THUMB.8 Load/store sign-extended byte/halfword
        int dst = instruction & 7;
        int base = (instruction >> 3) & 7;
        u32 address = m_reg[base] + m_reg[off];

        switch (op)
        {
        case 0b00: // STRH
            write_hword(address, m_reg[dst]);
            break;
        case 0b01: // LDSB
            m_reg[dst] = bus_read_byte(address);

            if (m_reg[dst] & 0x80)
                m_reg[dst] |= 0xFFFFFF00;
            break;
        case 0b10: // LDRH
            m_reg[dst] = read_hword(address);
            break;
        case 0b11: // LDSH
            m_reg[dst] = read_hword_signed(address);
            break;
        }
    }

    template <int op, int imm>
    void arm::thumb_9(u16 instruction)
    {
        // THUMB.9 Load store with immediate offset
        int dst = instruction & 7;
        int base = (instruction >> 3) & 7;

        switch (op)
        {
        case 0b00: { // STR
            u32 address = m_reg[base] + (imm << 2);
            write_word(address, m_reg[dst]);
            break;
        }
        case 0b01: { // LDR
            u32 address = m_reg[base] + (imm << 2);
            m_reg[dst] = read_word_rotated(address);
            break;
        }
        case 0b10: { // STRB
            u32 address = m_reg[base] + imm;
            bus_write_byte(address, m_reg[dst]);
            break;
        }
        case 0b11: { // LDRB
            u32 address = m_reg[base] + imm;
            m_reg[dst] = bus_read_byte(address);
            break;
        }
        }
    }

    template <bool load, int imm>
    void arm::thumb_10(u16 instruction)
    {
        // THUMB.10 Load/store halfword
        int dst = instruction & 7;
        int base = (instruction >> 3) & 7;
        u32 address = m_reg[base] + (imm << 1);

        if (load)
            m_reg[dst] = read_hword(address);
        else
            write_hword(address, m_reg[dst]);
    }

    template <bool load, int dst>
    void arm::thumb_11(u16 instruction)
    {
        // THUMB.11 SP-relative load/store
        u32 immediate_value = instruction & 0xFF;
        u32 address = m_reg[13] + (immediate_value << 2);

        if (load)
            m_reg[dst] = read_word_rotated(address);
        else
            write_word(address, m_reg[dst]);
    }

    template <bool stackptr, int dst>
    void arm::thumb_12(u16 instruction)
    {
        // THUMB.12 Load address
        u32 source_value = stackptr ? m_reg[13] : (m_reg[15] & ~2);
        u32 immediate_value = (instruction & 0xFF) << 2;

        m_reg[dst] = source_value + immediate_value;
    }

    template <bool sub>
    void arm::thumb_13(u16 instruction)
    {
        // THUMB.13 Add offset to stack pointer
        u32 immediate_value = (instruction & 0x7F) << 2;

        m_reg[13] += sub ? -immediate_value : immediate_value;
    }

    template <bool pop, bool rbit>
    void arm::thumb_14(u16 instruction)
    {
        // THUMB.14 push/pop registers
        u32 addr = m_reg[13];
        const int register_list = instruction & 0xFF;

        // hardware corner case. not sure if emulated correctly.
        if (!rbit && register_list == 0)
        {
            if (pop)
            {
                m_reg[15] = read_word(addr);
                m_pipeline.m_needs_flush = true;
            }
            else
            {
                write_word(addr, m_reg[15]);
            }
            m_reg[13] += pop ? 64 : -64;
            return;
        }

        if (!pop) // i.e. push
        {
            int register_count = 0;

            for (int i = 0; i <= 7; i++)
            {
                if (register_list & (1 << i))
                    register_count++;
            }

            if (rbit) register_count++;

            addr -= register_count * 4;
            m_reg[13] = addr;
        }

        // perform load/store multiple
        for (int i = 0; i <= 7; i++)
        {
            if (register_list & (1 << i))
            {
                if (pop)
                    m_reg[i] = read_word(addr);
                else
                    write_word(addr, m_reg[i]);
                addr += 4;
            }
        }

        if (rbit)
        {
            if (pop)
            {
                m_reg[15] = read_word(addr) & ~1;
                m_pipeline.m_needs_flush = true;
            }
            else
            {
                write_word(addr, m_reg[14]);
            }
            addr += 4;
        }

        if (pop) m_reg[13] = addr;
    }

    template <bool load, int base>
    void arm::thumb_15(u16 instruction)
    {
        // THUMB.15 Multiple load/store
        bool write_back = true;
        u32 address = m_reg[base];
        int register_list = instruction & 0xFF;

        if (load)
        {
            if (register_list == 0)
            {
                m_reg[15] = read_word(address);
                m_pipeline.m_needs_flush = true;
                m_reg[base] += 64;
                return;
            }

            for (int i = 0; i <= 7; i++)
            {
                if (register_list & (1<<i))
                {
                    m_reg[i] = read_word(address);
                    address += 4;
                }
            }

            if (write_back && (~register_list & (1<<base)))
            {
                m_reg[base] = address;
            }
        }
        else
        {
            int first_register = -1;

            if (register_list == 0)
            {
                write_word(address, m_reg[15]);
                m_reg[base] += 64;
                return;
            }

            for (int i = 0; i <= 7; i++)
            {
                if (register_list & (1<<i))
                {
                    if (first_register == -1) first_register = i;

                    if (i == base && i == first_register)
                    {
                        write_word(m_reg[base], address);
                    }
                    else
                    {
                        write_word(m_reg[base], m_reg[i]);
                    }

                    m_reg[base] += 4;
                }
            }
        }
    }

    template <int cond>
    void arm::thumb_16(u16 instruction)
    {
        // THUMB.16 Conditional branch
        if (check_condition(static_cast<cpu_condition>(cond)))
        {
            u32 signed_immediate = instruction & 0xFF;

            // sign-extend the immediate value if neccessary
            if (signed_immediate & 0x80)
                signed_immediate |= 0xFFFFFF00;

            // update r15/pc and flush pipe
            m_reg[15] += (signed_immediate << 1);
            m_pipeline.m_needs_flush = true;
        }
    }

    void arm::thumb_17(u16 instruction)
    {
        // THUMB.17 Software Interrupt
        u8 call_number = bus_read_byte(m_reg[15] - 4);

        if (!m_hle)
        {
            // save return address and program status
            m_bank[BANK_SVC][BANK_R14] = m_reg[15] - 2;
            m_spsr[SPSR_SVC] = m_cpsr;

            // switch to SVC mode and disable interrupts
            switch_mode(MODE_SVC);
            m_cpsr = (m_cpsr & ~MASK_THUMB) | MASK_IRQD;

            // jump to exception vector
            m_reg[15] = EXCPT_SWI;
            m_pipeline.m_needs_flush = true;
        }
        else
        {
            software_interrupt(call_number);
        }
    }

    void arm::thumb_18(u16 instruction)
    {
        // THUMB.18 Unconditional branch
        u32 immediate_value = (instruction & 0x3FF) << 1;

        // sign-extend r15/pc displacement
        if (instruction & 0x400)
            immediate_value |= 0xFFFFF800;

        // update r15/pc and flush pipe
        m_reg[15] += immediate_value;
        m_pipeline.m_needs_flush = true;
    }

    template <bool second_instruction>
    void arm::thumb_19(u16 instruction)
    {
        // THUMB.19 Long branch with link.
        u32 immediate_value = instruction & 0x7FF;

        if (second_instruction)
        {
            u32 temp_pc = m_reg[15] - 2;
            u32 value = m_reg[14] + (immediate_value << 1);

            // update r15/pc
            value &= 0x7FFFFF;
            m_reg[15] &= ~0x7FFFFF;
            m_reg[15] |= value & ~1;

            // store return address and flush pipe.
            m_reg[14] = temp_pc | 1;
            m_pipeline.m_needs_flush = true;
        }
        else
        {
            m_reg[14] = m_reg[15] + (immediate_value << 12);
        }
    }
}
