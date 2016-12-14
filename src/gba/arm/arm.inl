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

#pragma once

namespace GBA
{
    inline void arm::update_sign(u32 result)
    {
        m_state.m_cpsr = result & 0x80000000 ? (m_state.m_cpsr | MASK_NFLAG) : (m_state.m_cpsr & ~MASK_NFLAG);
    }

    inline void arm::update_zero(u64 result)
    {
        m_state.m_cpsr = result == 0 ? (m_state.m_cpsr | MASK_ZFLAG) : (m_state.m_cpsr & ~MASK_ZFLAG);
    }

    inline void arm::set_carry(bool carry)
    {
        m_state.m_cpsr = carry ? (m_state.m_cpsr | MASK_CFLAG) : (m_state.m_cpsr & ~MASK_CFLAG);
    }

    inline void arm::update_overflow_add(u32 result, u32 operand1, u32 operand2)
    {
        bool overflow = !(((operand1) ^ (operand2)) >> 31) && ((result) ^ (operand2)) >> 31;
        m_state.m_cpsr = overflow ? (m_state.m_cpsr | MASK_VFLAG) : (m_state.m_cpsr & ~MASK_VFLAG);
    }

    inline void arm::update_overflow_sub(u32 result, u32 operand1, u32 operand2)
    {
        bool overflow = ((operand1) ^ (operand2)) >> 31 && !(((result) ^ (operand2)) >> 31);
        m_state.m_cpsr = overflow ? (m_state.m_cpsr | MASK_VFLAG) : (m_state.m_cpsr & ~MASK_VFLAG);
    }

    inline void arm::logical_shift_left(u32& operand, u32 amount, bool& carry)
    {
        if (amount == 0) return;

        for (u32 i = 0; i < amount; i++)
        {
            carry = operand & 0x80000000 ? true : false;
            operand <<= 1;
        }
    }

    inline void arm::logical_shift_right(u32& operand, u32 amount, bool& carry, bool immediate)
    {
        // LSR #0 equals to LSR #32
        amount = immediate & (amount == 0) ? 32 : amount;

        for (u32 i = 0; i < amount; i++)
        {
            carry = operand & 1 ? true : false;
            operand >>= 1;
        }
    }

    inline void arm::arithmetic_shift_right(u32& operand, u32 amount, bool& carry, bool immediate)
    {
        u32 sign_bit = operand & 0x80000000;

        // ASR #0 equals to ASR #32
        amount = (immediate && (amount == 0)) ? 32 : amount;

        for (u32 i = 0; i < amount; i++)
        {
            carry = operand & 1 ? true : false;
            operand = (operand >> 1) | sign_bit;
        }
    }

    inline void arm::rotate_right(u32& operand, u32 amount, bool& carry, bool immediate)
    {
        // ROR #0 equals to RRX #1
        if (amount != 0 || !immediate)
        {
            for (u32 i = 1; i <= amount; i++)
            {
                u32 high_bit = (operand & 1) ? 0x80000000 : 0;
                operand = (operand >> 1) | high_bit;
                carry = high_bit == 0x80000000;
            }
        }
        else
        {
            bool old_carry = carry;
            carry = (operand & 1) ? true : false;
            operand = (operand >> 1) | (old_carry ? 0x80000000 : 0);
        }
    }

    inline u8 arm::ReadByte(u32 offset)
    {
        return Memory::ReadByte(offset);
    }

    inline u32 arm::ReadHWord(u32 offset)
    {
        if (offset & 1)
        {
            u32 value = Memory::ReadHWord(offset & ~1);

            return (value >> 8) | (value << 24);
        }

        return Memory::ReadHWord(offset);
    }

    inline u32 arm::ReadHWordSigned(u32 offset)
    {
        u32 value = 0;

        if (offset & 1)
        {
            value = Memory::ReadByte(offset);

            if (value & 0x80) value |= 0xFFFFFF00;
        }
        else
        {
            value = Memory::ReadHWord(offset);

            if (value & 0x8000) value |= 0xFFFF0000;
        }

        return value;
    }

    inline u32 arm::ReadWord(u32 offset)
    {
        return Memory::ReadWord(offset & ~3);
    }

    inline u32 arm::ReadWordRotated(u32 offset)
    {
        u32 value = ReadWord(offset);
        int amount = (offset & 3) * 8;

        return amount == 0 ? value : ((value >> amount) | (value << (32 - amount)));
    }

    inline void arm::WriteByte(u32 offset, u8 value)
    {
        Memory::WriteByte(offset, value);
    }

    inline void arm::WriteHWord(u32 offset, u16 value)
    {
        Memory::WriteHWord(offset & ~1, value);
    }

    inline void arm::WriteWord(u32 offset, u32 value)
    {
        Memory::WriteWord(offset & ~3, value);
    }

    inline void arm::RefillPipeline()
    {
        if (m_state.m_cpsr & MASK_THUMB)
        {
            m_state.m_pipeline.m_opcode[0] = Memory::ReadHWord(m_state.m_reg[15]);
            m_state.m_pipeline.m_opcode[1] = Memory::ReadHWord(m_state.m_reg[15] + 2);
            m_state.m_reg[15] += 4;
        }
        else
        {
            m_state.m_pipeline.m_opcode[0] = Memory::ReadWord(m_state.m_reg[15]);
            m_state.m_pipeline.m_opcode[1] = Memory::ReadWord(m_state.m_reg[15] + 4);
            m_state.m_reg[15] += 8;
        }
    }
}
