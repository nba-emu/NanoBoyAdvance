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


#include "arm.h"


namespace NanoboyAdvance
{
    const ARM7::ThumbInstruction ARM7::thumb_table[1024] =
    {
        /* THUMB.1 Move shifted register */
        &Thumb1<0,0>,  &Thumb1<1,0>,  &Thumb1<2,0>,  &Thumb1<3,0>,
        &Thumb1<4,0>,  &Thumb1<5,0>,  &Thumb1<6,0>,  &Thumb1<7,0>,
        &Thumb1<8,0>,  &Thumb1<9,0>,  &Thumb1<10,0>, &Thumb1<11,0>,
        &Thumb1<12,0>, &Thumb1<13,0>, &Thumb1<14,0>, &Thumb1<15,0>,
        &Thumb1<16,0>, &Thumb1<17,0>, &Thumb1<18,0>, &Thumb1<19,0>,
        &Thumb1<20,0>, &Thumb1<21,0>, &Thumb1<22,0>, &Thumb1<23,0>,
        &Thumb1<24,0>, &Thumb1<25,0>, &Thumb1<26,0>, &Thumb1<27,0>,
        &Thumb1<28,0>, &Thumb1<29,0>, &Thumb1<30,0>, &Thumb1<31,0>,
        &Thumb1<0,1>,  &Thumb1<1,1>,  &Thumb1<2,1>,  &Thumb1<3,1>,
        &Thumb1<4,1>,  &Thumb1<5,1>,  &Thumb1<6,1>,  &Thumb1<7,1>,
        &Thumb1<8,1>,  &Thumb1<9,1>,  &Thumb1<10,1>, &Thumb1<11,1>,
        &Thumb1<12,1>, &Thumb1<13,1>, &Thumb1<14,1>, &Thumb1<15,1>,
        &Thumb1<16,1>, &Thumb1<17,1>, &Thumb1<18,1>, &Thumb1<19,1>,
        &Thumb1<20,1>, &Thumb1<21,1>, &Thumb1<22,1>, &Thumb1<23,1>,
        &Thumb1<24,1>, &Thumb1<25,1>, &Thumb1<26,1>, &Thumb1<27,1>,
        &Thumb1<28,1>, &Thumb1<29,1>, &Thumb1<30,1>, &Thumb1<31,1>,
        &Thumb1<0,2>,  &Thumb1<1,2>,  &Thumb1<2,2>,  &Thumb1<3,2>,
        &Thumb1<4,2>,  &Thumb1<5,2>,  &Thumb1<6,2>,  &Thumb1<7,2>,
        &Thumb1<8,2>,  &Thumb1<9,2>,  &Thumb1<10,2>, &Thumb1<11,2>,
        &Thumb1<12,2>, &Thumb1<13,2>, &Thumb1<14,2>, &Thumb1<15,2>,
        &Thumb1<16,2>, &Thumb1<17,2>, &Thumb1<18,2>, &Thumb1<19,2>,
        &Thumb1<20,2>, &Thumb1<21,2>, &Thumb1<22,2>, &Thumb1<23,2>,
        &Thumb1<24,2>, &Thumb1<25,2>, &Thumb1<26,2>, &Thumb1<27,2>,
        &Thumb1<28,2>, &Thumb1<29,2>, &Thumb1<30,2>, &Thumb1<31,2>,

        /* THUMB.2 Add / subtract */
        &Thumb2<false, false, 0>, &Thumb2<false, false, 1>, &Thumb2<false, false, 2>, &Thumb2<false, false, 3>,
        &Thumb2<false, false, 4>, &Thumb2<false, false, 5>, &Thumb2<false, false, 6>, &Thumb2<false, false, 7>,
        &Thumb2<false, true, 0>,  &Thumb2<false, true, 1>,  &Thumb2<false, true, 2>,  &Thumb2<false, true, 3>,
        &Thumb2<false, true, 4>,  &Thumb2<false, true, 5>,  &Thumb2<false, true, 6>,  &Thumb2<false, true, 7>,
        &Thumb2<true, false, 0>,  &Thumb2<true, false, 1>,  &Thumb2<true, false, 2>,  &Thumb2<true, false, 3>,
        &Thumb2<true, false, 4>,  &Thumb2<true, false, 5>,  &Thumb2<true, false, 6>,  &Thumb2<true, false, 7>,
        &Thumb2<true, true, 0>,   &Thumb2<true, true, 1>,   &Thumb2<true, true, 2>,   &Thumb2<true, true, 3>,
        &Thumb2<true, true, 4>,   &Thumb2<true, true, 5>,   &Thumb2<true, true, 6>,   &Thumb2<true, true, 7>,

        &Thumb3, &Thumb3,
        &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3,
        &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3,
        &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3,
        &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3,
        &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3,
        &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3,
        &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3,
        &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3,
        &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3,
        &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3,
        &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3,
        &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3,
        &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb3, &Thumb4, &Thumb4, &Thumb4, &Thumb4,
        &Thumb4, &Thumb4, &Thumb4, &Thumb4, &Thumb4, &Thumb4, &Thumb4, &Thumb4, &Thumb4, &Thumb4,
        &Thumb4, &Thumb4, &Thumb5, &Thumb5, &Thumb5, &Thumb5, &Thumb5, &Thumb5, &Thumb5, &Thumb5,
        &Thumb5, &Thumb5, &Thumb5, &Thumb5, &Thumb5, &Thumb5, &Thumb5, &Thumb5, &Thumb6, &Thumb6,
        &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6,
        &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6,
        &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6, &Thumb6,
        &Thumb7, &Thumb7, &Thumb7, &Thumb7, &Thumb7, &Thumb7, &Thumb7, &Thumb7, &Thumb8, &Thumb8,
        &Thumb8, &Thumb8, &Thumb8, &Thumb8, &Thumb8, &Thumb8, &Thumb7, &Thumb7, &Thumb7, &Thumb7,
        &Thumb7, &Thumb7, &Thumb7, &Thumb7, &Thumb8, &Thumb8, &Thumb8, &Thumb8, &Thumb8, &Thumb8,
        &Thumb8, &Thumb8, &Thumb7, &Thumb7, &Thumb7, &Thumb7, &Thumb7, &Thumb7, &Thumb7, &Thumb7,
        &Thumb8, &Thumb8, &Thumb8, &Thumb8, &Thumb8, &Thumb8, &Thumb8, &Thumb8, &Thumb7, &Thumb7,
        &Thumb7, &Thumb7, &Thumb7, &Thumb7, &Thumb7, &Thumb7, &Thumb8, &Thumb8, &Thumb8, &Thumb8,
        &Thumb8, &Thumb8, &Thumb8, &Thumb8, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9,
        &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9,
        &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9,
        &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9,
        &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9,
        &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9,
        &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9,
        &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9,
        &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9,
        &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9,
        &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9,
        &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9,
        &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9, &Thumb9,
        &Thumb9, &Thumb9, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10,
        &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10,
        &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10,
        &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10,
        &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10,
        &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10,
        &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb10, &Thumb11, &Thumb11, &Thumb11, &Thumb11,
        &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11,
        &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11,
        &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11,
        &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11,
        &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11,
        &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11, &Thumb11,
        &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12,
        &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12,
        &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12,
        &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12,
        &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12,
        &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb12,
        &Thumb12, &Thumb12, &Thumb12, &Thumb12, &Thumb13, &Thumb13, &Thumb13, &Thumb13, &Thumb13, &Thumb13,
        &Thumb13, &Thumb13, &Thumb13, &Thumb13, &Thumb13, &Thumb13, &Thumb13, &Thumb13, &Thumb13, &Thumb13,
        &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14,
        &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb13, &Thumb13, &Thumb13, &Thumb13,
        &Thumb13, &Thumb13, &Thumb13, &Thumb13, &Thumb13, &Thumb13, &Thumb13, &Thumb13, &Thumb13, &Thumb13,
        &Thumb13, &Thumb13, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14,
        &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb14, &Thumb15, &Thumb15,
        &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15,
        &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15,
        &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15,
        &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15,
        &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15,
        &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15, &Thumb15,
        &Thumb15, &Thumb15, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16,
        &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16,
        &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16,
        &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16,
        &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16,
        &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16, &Thumb16,
        &Thumb16, &Thumb16, &Thumb17, &Thumb17, &Thumb17, &Thumb17, &Thumb18, &Thumb18, &Thumb18, &Thumb18,
        &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18,
        &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18,
        &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18,
        &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18,
        &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18,
        &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18, &Thumb18,
        &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19,
        &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19,
        &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19,
        &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19,
        &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19,
        &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19, &Thumb19,
        &Thumb19, &Thumb19, &Thumb19, &Thumb19
    };

    template <int imm, int type>
    void ARM7::Thumb1(u16 instruction)
    {
        // THUMB.1 Move shifted register
        int reg_dest = instruction & 7;
        int reg_source = (instruction >> 3) & 7;
        bool carry = cpsr & CarryFlag;

        reg(reg_dest) = reg(reg_source);

        switch (type)
        {
        case 0: // LSL
            LSL(reg(reg_dest), imm, carry);
            AssertCarry(carry);
            break;
        case 1: // LSR
            LSR(reg(reg_dest), imm, carry, true);
            AssertCarry(carry);
            break;
        case 2: // ASR
        {
            ASR(reg(reg_dest), imm, carry, true);
            AssertCarry(carry);
            break;
        }
        }

        // Update sign and zero flag
        CalculateSign(reg(reg_dest));
        CalculateZero(reg(reg_dest));

        // Update cycle counter
        cycles += memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD);
    }

    template <bool immediate, bool subtract, int field3>
    void ARM7::Thumb2(u16 instruction)
    {
        // THUMB.2 Add/subtract
        int reg_dest = instruction & 7;
        int reg_source = (instruction >> 3) & 7;
        u32 operand;

        // Either a register or an immediate
        if (immediate)
            operand = field3;
        else
            operand = reg(field3);

        // Determine wether to subtract or add
        if (subtract)
        {
            u32 result = reg(reg_source) - operand;

            // Calculate flags
            AssertCarry(reg(reg_source) >= operand);
            CalculateOverflowSub(result, reg(reg_source), operand);
            CalculateSign(result);
            CalculateZero(result);

            reg(reg_dest) = result;
        }
        else
        {
            u32 result = reg(reg_source) + operand;
            u64 result_long = (u64)(reg(reg_source)) + (u64)operand;

            // Calculate flags
            AssertCarry(result_long & 0x100000000);
            CalculateOverflowAdd(result, reg(reg_source), operand);
            CalculateSign(result);
            CalculateZero(result);

            reg(reg_dest) = result;
        }

        // Update cycle counter
        cycles += memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD);
    }

    void ARM7::Thumb3(u16 instruction)
    {
        // THUMB.3 Move/compare/add/subtract immediate
        u32 immediate_value = instruction & 0xFF;
        int reg_dest = (instruction >> 8) & 7;

        switch ((instruction >> 11) & 3)
        {
        case 0b00: // MOV
            CalculateSign(0);
            CalculateZero(immediate_value);
            reg(reg_dest) = immediate_value;
            break;
        case 0b01: // CMP
        {
            u32 result = reg(reg_dest) - immediate_value;
            AssertCarry(reg(reg_dest) >= immediate_value);
            CalculateOverflowSub(result, reg(reg_dest), immediate_value);
            CalculateSign(result);
            CalculateZero(result);
            break;
        }
        case 0b10: // ADD
        {
            u32 result = reg(reg_dest) + immediate_value;
            u64 result_long = (u64)(reg(reg_dest)) + (u64)immediate_value;
            AssertCarry(result_long & 0x100000000);
            CalculateOverflowAdd(result, reg(reg_dest), immediate_value);
            CalculateSign(result);
            CalculateZero(result);
            reg(reg_dest) = result;
            break;
        }
        case 0b11: // SUB
        {
            u32 result = reg(reg_dest) - immediate_value;
            AssertCarry(reg(reg_dest) >= immediate_value);
            CalculateOverflowSub(result, reg(reg_dest), immediate_value);
            CalculateSign(result);
            CalculateZero(result);
            reg(reg_dest) = result;
            break;
        }
        }

        // Update cycle counter
        cycles += memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD);
    }

    void ARM7::Thumb4(u16 instruction)
    {
        // THUMB.4 ALU operations
        int reg_dest = instruction & 7;
        int reg_source = (instruction >> 3) & 7;

        switch ((instruction >> 6) & 0xF)
        {
        case 0b0000: // AND
            reg(reg_dest) &= reg(reg_source);
            CalculateSign(reg(reg_dest));
            CalculateZero(reg(reg_dest));
            break;
        case 0b0001: // EOR
            reg(reg_dest) ^= reg(reg_source);
            CalculateSign(reg(reg_dest));
            CalculateZero(reg(reg_dest));
            break;
        case 0b0010: // LSL
        {
            u32 amount = reg(reg_source);
            bool carry = cpsr & CarryFlag;
            LSL(reg(reg_dest), amount, carry);
            AssertCarry(carry);
            CalculateSign(reg(reg_dest));
            CalculateZero(reg(reg_dest));
            cycles++;
            break;
        }
        case 0b0011: // LSR
        {
            u32 amount = reg(reg_source);
            bool carry = cpsr & CarryFlag;
            LSR(reg(reg_dest), amount, carry, false);
            AssertCarry(carry);
            CalculateSign(reg(reg_dest));
            CalculateZero(reg(reg_dest));
            cycles++;
            break;
        }
        case 0b0100: // ASR
        {
            u32 amount = reg(reg_source);
            bool carry = cpsr & CarryFlag;
            ASR(reg(reg_dest), amount, carry, false);
            AssertCarry(carry);
            CalculateSign(reg(reg_dest));
            CalculateZero(reg(reg_dest));
            cycles++;
            break;
        }
        case 0b0101: // ADC
        {
            int carry = (cpsr >> 29) & 1;
            u32 result = reg(reg_dest) + reg(reg_source) + carry;
            u64 result_long = (u64)(reg(reg_dest)) + (u64)(reg(reg_source)) + (u64)carry;
            AssertCarry(result_long & 0x100000000);
            CalculateOverflowAdd(result, reg(reg_dest), reg(reg_source));
            CalculateSign(result);
            CalculateZero(result);
            reg(reg_dest) = result;
            break;
        }
        case 0b0110: // SBC
        {
            int carry = (cpsr >> 29) & 1;
            u32 result = reg(reg_dest) - reg(reg_source) + carry - 1;
            AssertCarry(reg(reg_dest) >= (reg(reg_source) + carry - 1));
            CalculateOverflowSub(result, reg(reg_dest), reg(reg_source));
            CalculateSign(result);
            CalculateZero(result);
            reg(reg_dest) = result;
            break;
        }
        case 0b0111: // ROR
        {
            u32 amount = reg(reg_source);
            bool carry = cpsr & CarryFlag;
            ROR(reg(reg_dest), amount, carry, false);
            AssertCarry(carry);
            CalculateSign(reg(reg_dest));
            CalculateZero(reg(reg_dest));
            cycles++;
            break;
        }
        case 0b1000: // TST
        {
            u32 result = reg(reg_dest) & reg(reg_source);
            CalculateSign(result);
            CalculateZero(result);
            break;
        }
        case 0b1001: // NEG
        {
            u32 result = 0 - reg(reg_source);
            AssertCarry(0 >= reg(reg_source));
            CalculateOverflowSub(result, 0, reg(reg_source));
            CalculateSign(result);
            CalculateZero(result);
            reg(reg_dest) = result;
            break;
        }
        case 0b1010: // CMP
        {
            u32 result = reg(reg_dest) - reg(reg_source);
            AssertCarry(reg(reg_dest) >= reg(reg_source));
            CalculateOverflowSub(result, reg(reg_dest), reg(reg_source));
            CalculateSign(result);
            CalculateZero(result);
            break;
        }
        case 0b1011: // CMN
        {
            u32 result = reg(reg_dest) + reg(reg_source);
            u64 result_long = (u64)(reg(reg_dest)) + (u64)(reg(reg_source));
            AssertCarry(result_long & 0x100000000);
            CalculateOverflowAdd(result, reg(reg_dest), reg(reg_source));
            CalculateSign(result);
            CalculateZero(result);
            break;
        }
        case 0b1100: // ORR
            reg(reg_dest) |= reg(reg_source);
            CalculateSign(reg(reg_dest));
            CalculateZero(reg(reg_dest));
            break;
        case 0b1101: // MUL
            // TODO: how to calc. the internal cycles?
            reg(reg_dest) *= reg(reg_source);
            CalculateSign(reg(reg_dest));
            CalculateZero(reg(reg_dest));
            AssertCarry(false);
            break;
        case 0b1110: // BIC
            reg(reg_dest) &= ~(reg(reg_source));
            CalculateSign(reg(reg_dest));
            CalculateZero(reg(reg_dest));
            break;
        case 0b1111: // MVN
            reg(reg_dest) = ~(reg(reg_source));
            CalculateSign(reg(reg_dest));
            CalculateZero(reg(reg_dest));
            break;
        }

        // Update cycle counter
        cycles += memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD);
    }

    void ARM7::Thumb5(u16 instruction)
    {
        // THUMB.5 Hi register operations/branch exchange
        int reg_dest = instruction & 7;
        int reg_source = (instruction >> 3) & 7;
        int opcode = (instruction >> 8) & 3;
        bool compare = false;
        u32 operand;

        // Both reg_dest and reg_source can encode either a low register (r0-r7) or a high register (r8-r15)
        switch ((instruction >> 6) & 3)
        {
        case 0b01:
            reg_source += 8;
            break;
        case 0b10:
            reg_dest += 8;
            break;
        case 0b11:
            reg_dest += 8;
            reg_source += 8;
            break;
        }

        operand = reg(reg_source);

        if (reg_source == 15)
            operand &= ~1;

        // Time next pipeline prefetch
        cycles += memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD);

        // Perform the actual operation
        switch (opcode)
        {
        case 0b00: // ADD
            reg(reg_dest) += operand;
            break;
        case 0b01: // CMP
        {
            u32 result = reg(reg_dest) - operand;
            AssertCarry(reg(reg_dest) >= operand);
            CalculateOverflowSub(result, reg(reg_dest), operand);
            CalculateSign(result);
            CalculateZero(result);
            compare = true;
            break;
        }
        case 0b10: // MOV
            reg(reg_dest) = operand;
            break;
        case 0b11: // BX
            // Bit0 being set in the address indicates
            // that the destination instruction is in THUMB mode.
            if (operand & 1)
            {
                r[15] = operand & ~1;

                // Emulate pipeline refill cycles
                cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                        memory->SequentialAccess(r[15] + 2, GBAMemory::ACCESS_HWORD);
            }
            else
            {
                cpsr &= ~ThumbFlag;
                r[15] = operand & ~3;

                // Emulate pipeline refill cycles
                cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_WORD) +
                        memory->SequentialAccess(r[15] + 4, GBAMemory::ACCESS_WORD);
            }

            // Flush pipeline
            pipe.flush = true;
            break;
        }

        if (reg_dest == 15 && !compare && opcode != 0b11)
        {
            // Flush pipeline
            reg(reg_dest) &= ~1;
            pipe.flush = true;

            // Emulate pipeline refill cycles
            cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                    memory->SequentialAccess(r[15] + 2, GBAMemory::ACCESS_HWORD);
        }
    }

    void ARM7::Thumb6(u16 instruction)
    {
        // THUMB.6 PC-relative load
        u32 immediate_value = instruction & 0xFF;
        int reg_dest = (instruction >> 8) & 7;
        u32 address = (r[15] & ~2) + (immediate_value << 2);

        reg(reg_dest) = ReadWord(address);

        cycles += 1 + memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
        memory->NonSequentialAccess(address, GBAMemory::ACCESS_WORD);
    }

    void ARM7::Thumb7(u16 instruction)
    {
        // THUMB.7 Load/store with register offset
        // TODO: check LDR(B) timings.
        int reg_dest = instruction & 7;
        int reg_base = (instruction >> 3) & 7;
        int reg_offset = (instruction >> 6) & 7;
        u32 address = reg(reg_base) + reg(reg_offset);

        switch ((instruction >> 10) & 3)
        {
        case 0b00: // STR
            WriteWord(address, reg(reg_dest));
            cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                    memory->NonSequentialAccess(address, GBAMemory::ACCESS_WORD);
            break;
        case 0b01: // STRB
            WriteByte(address, reg(reg_dest) & 0xFF);
            cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                    memory->NonSequentialAccess(address, GBAMemory::ACCESS_BYTE);
            break;
        case 0b10: // LDR
            reg(reg_dest) = ReadWordRotated(address);
            cycles += 1 + memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                        memory->NonSequentialAccess(address, GBAMemory::ACCESS_WORD);
            break;
        case 0b11: // LDRB
            reg(reg_dest) = ReadByte(address);
            cycles += 1 + memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                        memory->NonSequentialAccess(address, GBAMemory::ACCESS_BYTE);
            break;
        }
    }

    void ARM7::Thumb8(u16 instruction)
    {
        // THUMB.8 Load/store sign-extended byte/halfword
        int reg_dest = instruction & 7;
        int reg_base = (instruction >> 3) & 7;
        int reg_offset = (instruction >> 6) & 7;
        u32 address = reg(reg_base) + reg(reg_offset);

        switch ((instruction >> 10) & 3)
        {
        case 0b00: // STRH
            WriteHWord(address, reg(reg_dest));
            cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                    memory->NonSequentialAccess(address, GBAMemory::ACCESS_HWORD);
            break;
        case 0b01: // LDSB
            reg(reg_dest) = ReadByte(address);

            if (reg(reg_dest) & 0x80)
                reg(reg_dest) |= 0xFFFFFF00;

            cycles += 1 + memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                        memory->NonSequentialAccess(address, GBAMemory::ACCESS_BYTE);
            break;
        case 0b10: // LDRH
            reg(reg_dest) = ReadHWord(address);
            cycles += 1 + memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                        memory->NonSequentialAccess(address, GBAMemory::ACCESS_HWORD);
            break;
        case 0b11: // LDSH
            reg(reg_dest) = ReadHWordSigned(address);

            // Uff... we should check wether ReadHWordSigned reads a
            // byte or a hword. However this should never really make difference.
            cycles += 1 + memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                        memory->NonSequentialAccess(address, GBAMemory::ACCESS_HWORD);
            break;
        }
    }

    void ARM7::Thumb9(u16 instruction)
    {
        // THUMB.9 Load store with immediate offset
        int reg_dest = instruction & 7;
        int reg_base = (instruction >> 3) & 7;
        u32 immediate_value = (instruction >> 6) & 0x1F;

        switch ((instruction >> 11) & 3)
        {
        case 0b00: { // STR
            u32 address = reg(reg_base) + (immediate_value << 2);
            WriteWord(address, reg(reg_dest));
            cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                      memory->NonSequentialAccess(address, GBAMemory::ACCESS_WORD);
            break;
        }
        case 0b01: { // LDR
            u32 address = reg(reg_base) + (immediate_value << 2);
            reg(reg_dest) = ReadWordRotated(address);
            cycles += 1 + memory->SequentialAccess(r[15],  GBAMemory::ACCESS_HWORD) +
                          memory->NonSequentialAccess(address, GBAMemory::ACCESS_WORD);
            break;
        }
        case 0b10: { // STRB
            u32 address = reg(reg_base) + immediate_value;
            WriteByte(address, reg(reg_dest));
            cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                      memory->NonSequentialAccess(address, GBAMemory::ACCESS_BYTE);
            break;
        }
        case 0b11: { // LDRB
            u32 address = reg(reg_base) + immediate_value;
            reg(reg_dest) = ReadByte(address);
            cycles += 1 + memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                          memory->NonSequentialAccess(address, GBAMemory::ACCESS_BYTE);
            break;
        }
        }
    }

    void ARM7::Thumb10(u16 instruction)
    {
        // THUMB.10 Load/store halfword
        int reg_dest = instruction & 7;
        int reg_base = (instruction >> 3) & 7;
        u32 immediate_value = (instruction >> 6) & 0x1F;
        u32 address = reg(reg_base) + (immediate_value << 1);

        // Is the load bit set? (ldr)
        if (instruction & (1 << 11))
        {
            reg(reg_dest) = ReadHWord(address); // todo: alignment?
            cycles += 1 + memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                          memory->NonSequentialAccess(address, GBAMemory::ACCESS_WORD);
        }
        else
        {
            WriteHWord(address, reg(reg_dest));
            cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                      memory->NonSequentialAccess(address, GBAMemory::ACCESS_WORD);
        }
    }

    void ARM7::Thumb11(u16 instruction)
    {
        // THUMB.11 SP-relative load/store
        u32 immediate_value = instruction & 0xFF;
        int reg_dest = (instruction >> 8) & 7;
        u32 address = reg(13) + (immediate_value << 2);

        // Is the load bit set? (ldr)
        if (instruction & (1 << 11))
        {
            reg(reg_dest) = ReadWordRotated(address);
            cycles += 1 + memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                          memory->NonSequentialAccess(address, GBAMemory::ACCESS_WORD);
        }
        else
        {
            WriteWord(address, reg(reg_dest));
            cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                      memory->NonSequentialAccess(address, GBAMemory::ACCESS_WORD);
        }
    }

    void ARM7::Thumb12(u16 instruction)
    {
        // THUMB.12 Load address
        u32 immediate_value = instruction & 0xFF;
        int reg_dest = (instruction >> 8) & 7;

        // Use stack pointer as base?
        if (instruction & (1 << 11))
            reg(reg_dest) = reg(13) + (immediate_value << 2); // sp
        else
            reg(reg_dest) = (r[15] & ~2) + (immediate_value << 2); // pc

        cycles += memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD);
    }

    void ARM7::Thumb13(u16 instruction)
    {
        // THUMB.13 Add offset to stack pointer
        u32 immediate_value = (instruction & 0x7F) << 2;

        // Immediate-value is negative?
        if (instruction & 0x80)
            reg(13) -= immediate_value;
        else
            reg(13) += immediate_value;

        cycles += memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD);
    }

    void ARM7::Thumb14(u16 instruction)
    {
        // THUMB.14 push/pop registers
        // TODO: how to handle an empty register list?
        bool first_access = true;

        // One non-sequential prefetch cycle
        cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD);

        // Is this a POP instruction?
        if (instruction & (1 << 11))
        {
            // Iterate through the entire register list
            for (int i = 0; i <= 7; i++)
            {
                // Pop into this register?
                if (instruction & (1 << i))
                {
                    u32 address = reg(13);

                    // Read word and update SP.
                    reg(i) = ReadWord(address);
                    reg(13) += 4;

                    // Time the access based on if it's a first access
                    if (first_access)
                    {
                        cycles += memory->NonSequentialAccess(address, GBAMemory::ACCESS_WORD);
                        first_access = false;
                    }
                    else
                    {
                        cycles += memory->SequentialAccess(address, GBAMemory::ACCESS_WORD);
                    }
                }
            }

            // Also pop r15/pc if neccessary
            if (instruction & (1 << 8))
            {
                u32 address = reg(13);

                // Read word and update SP.
                r[15] = ReadWord(reg(13)) & ~1;
                reg(13) += 4;

                // Time the access based on if it's a first access
                if (first_access)
                {
                    cycles += memory->NonSequentialAccess(address, GBAMemory::ACCESS_WORD);
                    first_access = false;
                }
                else
                {
                    cycles += memory->SequentialAccess(address, GBAMemory::ACCESS_WORD);
                }

                pipe.flush = true;
            }
        }
        else
        {
            // Push r14/lr if neccessary
            if (instruction & (1 << 8))
            {
                u32 address;

                // Write word and update SP.
                reg(13) -= 4;
                address = reg(13);
                WriteWord(address, reg(14));

                // Time the access based on if it's a first access
                if (first_access)
                {
                    cycles += memory->NonSequentialAccess(address, GBAMemory::ACCESS_WORD);
                    first_access = false;
                }
                else
                {
                    cycles += memory->SequentialAccess(address, GBAMemory::ACCESS_WORD);
                }
            }

            // Iterate through the entire register list
            for (int i = 7; i >= 0; i--)
            {
                // Push this register?
                if (instruction & (1 << i))
                {
                    u32 address;

                    // Write word and update SP.
                    reg(13) -= 4;
                    address = reg(13);
                    WriteWord(address, reg(i));

                    // Time the access based on if it's a first access
                    if (first_access)
                    {
                        cycles += memory->NonSequentialAccess(address, GBAMemory::ACCESS_WORD);
                        first_access = false;
                    }
                    else
                    {
                        cycles += memory->SequentialAccess(address, GBAMemory::ACCESS_WORD);
                    }
                }
            }
        }
    }

    void ARM7::Thumb15(u16 instruction)
    {
        // THUMB.15 Multiple load/store
        // TODO: Handle empty register list
        int reg_base = (instruction >> 8) & 7;
        bool write_back = true;
        u32 address = reg(reg_base);

        // Is the load bit set? (ldmia or stmia)
        if (instruction & (1 << 11))
        {
            cycles += 1 + memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                          memory->SequentialAccess(r[15] + 2, GBAMemory::ACCESS_HWORD);

            // Iterate through the entire register list
            for (int i = 0; i <= 7; i++)
            {
                // Load to this register?
                if (instruction & (1 << i))
                {
                    reg(i) = ReadWord(address);
                    cycles += memory->SequentialAccess(address, GBAMemory::ACCESS_WORD);
                    address += 4;
                }
            }

            // Write back address into the base register if specified
            // and the base register is not in the register list
            if (write_back && !(instruction & (1 << reg_base)))
                reg(reg_base) = address;
        }
        else
        {
            int first_register = 0;
            bool first_access = true;

            cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD);

            // Find the first register
            for (int i = 0; i < 8; i++)
            {
                if (instruction & (1 << i))
                {
                    first_register = i;
                    break;
                }
            }

            // Iterate through the entire register list
            for (int i = 0; i <= 7; i++)
            {
                // Store this register?
                if (instruction & (1 << i))
                {
                    // Write register to the base address. If the current register is the
                    // base register and also the first register instead the original base is written.
                    if (i == reg_base && i == first_register)
                        WriteWord(reg(reg_base), address);
                    else
                        WriteWord(reg(reg_base), reg(i));

                    // Time the access based on if it's a first access
                    if (first_access)
                    {
                        cycles += memory->NonSequentialAccess(reg(reg_base), GBAMemory::ACCESS_WORD);
                        first_access = false;
                    }
                    else
                    {
                        cycles += memory->SequentialAccess(reg(reg_base), GBAMemory::ACCESS_WORD);
                    }

                    // Update base address
                    reg(reg_base) += 4;
                }
            }
        }
    }

    void ARM7::Thumb16(u16 instruction)
    {
        // THUMB.16 Conditional branch
        u32 signed_immediate = instruction & 0xFF;

        cycles += memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD);

        // Check if the instruction will be executed
        switch ((instruction >> 8) & 0xF)
        {
        case 0x0: if (!(cpsr & ZeroFlag))     return; break;
        case 0x1: if (  cpsr & ZeroFlag)      return; break;
        case 0x2: if (!(cpsr & CarryFlag))    return; break;
        case 0x3: if (  cpsr & CarryFlag)     return; break;
        case 0x4: if (!(cpsr & SignFlag))     return; break;
        case 0x5: if (  cpsr & SignFlag)      return; break;
        case 0x6: if (!(cpsr & OverflowFlag)) return; break;
        case 0x7: if (  cpsr & OverflowFlag)  return; break;
        case 0x8: if (!(cpsr & CarryFlag) ||  (cpsr & ZeroFlag)) return; break;
        case 0x9: if ( (cpsr & CarryFlag) && !(cpsr & ZeroFlag)) return; break;
        case 0xA: if ((cpsr & SignFlag) != (cpsr & OverflowFlag)) return; break;
        case 0xB: if ((cpsr & SignFlag) == (cpsr & OverflowFlag)) return; break;
        case 0xC: if ((cpsr & ZeroFlag) || ((cpsr & SignFlag) != (cpsr & OverflowFlag))) return; break;
        case 0xD: if (!(cpsr & ZeroFlag) && ((cpsr & SignFlag) == (cpsr & OverflowFlag))) return; break;
        }

        // Sign-extend the immediate value if neccessary
        if (signed_immediate & 0x80)
            signed_immediate |= 0xFFFFFF00;

        // Update r15/pc and flush pipe
        r[15] += (signed_immediate << 1);
        pipe.flush = true;

        // Emulate pipeline refill timings
        cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                  memory->SequentialAccess(r[15] + 2, GBAMemory::ACCESS_HWORD);
    }

    void ARM7::Thumb17(u16 instruction)
    {
        // THUMB.17 Software Interrupt
        u8 bios_call = ReadByte(r[15] - 4);

        // Log SWI to the console
        #ifdef DEBUG
        LOG(LOG_INFO, "swi 0x%x r0=0x%x, r1=0x%x, r2=0x%x, r3=0x%x, lr=0x%x, pc=0x%x (thumb)",
            bios_call, r[0], r[1], r[2], r[3], reg(14), r[15]);
        #endif

        // "Useless" prefetch from r15 and pipeline refill timing.
        cycles += memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                memory->NonSequentialAccess(8, GBAMemory::ACCESS_WORD) +
                memory->SequentialAccess(12, GBAMemory::ACCESS_WORD);

        // Dispatch SWI, either HLE or BIOS.
        if (hle)
        {
            SWI(bios_call);
        }
        else
        {
            // Store return address in r14<svc>
            r14_svc = r[15] - 2;

            // Save program status and switch mode
            SaveRegisters();
            spsr_svc = cpsr;
            cpsr = (cpsr & ~(ModeField | ThumbFlag)) | (u32)Mode::SVC | IrqDisable;
            LoadRegisters();

            // Jump to exception vector
            r[15] = (u32)Exception::SoftwareInterrupt;
            pipe.flush = true;
        }
    }

    void ARM7::Thumb18(u16 instruction)
    {
        // THUMB.18 Unconditional branch
        u32 immediate_value = (instruction & 0x3FF) << 1;

        cycles += memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD);

        // Sign-extend r15/pc displacement
        if (instruction & 0x400)
            immediate_value |= 0xFFFFF800;

        // Update r15/pc and flush pipe
        r[15] += immediate_value;
        pipe.flush = true;

        //Emulate pipeline refill timings
        cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                  memory->SequentialAccess(r[15] + 2, GBAMemory::ACCESS_HWORD);
    }

    void ARM7::Thumb19(u16 instruction)
    {
        // THUMB.19 Long branch with link.
        u32 immediate_value = instruction & 0x7FF;

        // Branch with link consists of two instructions.
        if (instruction & (1 << 11))
        {
            u32 temp_pc = r[15] - 2;
            u32 value = reg(14) + (immediate_value << 1);

            cycles += memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD);

            // Update r15/pc
            value &= 0x7FFFFF;
            r[15] &= ~0x7FFFFF;
            r[15] |= value & ~1;

            // Store return address and flush pipe.
            reg(14) = temp_pc | 1;
            pipe.flush = true;

            //Emulate pipeline refill timings
            cycles += memory->NonSequentialAccess(r[15], GBAMemory::ACCESS_HWORD) +
                      memory->SequentialAccess(r[15] + 2, GBAMemory::ACCESS_HWORD);
        }
        else
        {
            reg(14) = r[15] + (immediate_value << 12);
            cycles += memory->SequentialAccess(r[15], GBAMemory::ACCESS_HWORD);
        }
    }

    void ARM7::ExecuteThumb(u16 instruction)
    {
        (*this.*thumb_table[instruction >> 6])(instruction);
    }
}
