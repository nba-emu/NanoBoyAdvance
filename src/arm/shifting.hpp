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

#ifdef ARMIGO_INCLUDE

inline void logical_shift_left(u32& operand, u32 amount, bool& carry)
{
    if (amount == 0) return;

    for (u32 i = 0; i < amount; i++)
    {
        carry = operand & 0x80000000 ? true : false;
        operand <<= 1;
    }
}

inline void logical_shift_right(u32& operand, u32 amount, bool& carry, bool immediate)
{
    // LSR #0 equals to LSR #32
    amount = immediate & (amount == 0) ? 32 : amount;

    for (u32 i = 0; i < amount; i++)
    {
        carry = operand & 1 ? true : false;
        operand >>= 1;
    }
}

inline void arithmetic_shift_right(u32& operand, u32 amount, bool& carry, bool immediate)
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

inline void rotate_right(u32& operand, u32 amount, bool& carry, bool immediate)
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

inline void perform_shift(int shift, u32& operand, u32 amount, bool& carry, bool immediate)
{
    switch (shift)
    {
    case 0:
        logical_shift_left(operand, amount, carry);
        return;
    case 1:
        logical_shift_right(operand, amount, carry, immediate);
        return;
    case 2:
        arithmetic_shift_right(operand, amount, carry, immediate);
        return;
    case 3:
        rotate_right(operand, amount, carry, immediate);
    }
}

#endif
