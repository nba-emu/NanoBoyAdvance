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

#include "util/integer.hpp"
#include "util/log.h"
#include "../memory.h"
#include "state.hpp"

namespace GBA
{
    class arm
    {
        typedef void (arm::*ThumbInstruction)(u16);
        static const ThumbInstruction THUMB_TABLE[1024];

        state m_state;

        struct
        {
            u32 m_Opcode[3];
            int m_Index;
            bool m_Flush;
        } m_Pipe;

        bool hle;

        void update_sign(u32 result);
        void update_zero(u64 result);
        void set_carry(bool carry);
        void update_overflow_add(u32 result, u32 operand1, u32 operand2);
        void update_overflow_sub(u32 result, u32 operand1, u32 operand2);
        static void logical_shift_left(u32& operand, u32 amount, bool& carry);
        static void logical_shift_right(u32& operand, u32 amount, bool& carry, bool immediate);
        static void arithmetic_shift_right(u32& operand, u32 amount, bool& carry, bool immediate);
        static void rotate_right(u32& operand, u32 amount, bool& carry, bool immediate);

        u8 ReadByte(u32 offset);
        u32 ReadHWord(u32 offset);
        u32 ReadHWordSigned(u32 offset);
        u32 ReadWord(u32 offset);
        u32 ReadWordRotated(u32 offset);
        void WriteByte(u32 offset, u8 value);
        void WriteHWord(u32 offset, u16 value);
        void WriteWord(u32 offset, u32 value);
        void RefillPipeline();

        // Thumb Instructions
        template <int imm, int type>
        void Thumb1(u16 instruction);

        template <bool immediate, bool subtract, int field3>
        void Thumb2(u16 instruction);

        template <int op, int reg_dest>
        void Thumb3(u16 instruction);

        template <int op>
        void Thumb4(u16 instruction);

        template <int op, bool high1, bool high2>
        void Thumb5(u16 instruction);

        template <int reg_dest>
        void Thumb6(u16 instruction);

        template <int op, int reg_offset>
        void Thumb7(u16 instruction);

        template <int op, int reg_offset>
        void Thumb8(u16 instruction);

        template <int op, int imm>
        void Thumb9(u16 instruction);

        template <bool load, int imm>
        void Thumb10(u16 instruction);

        template <bool load, int reg_dest>
        void Thumb11(u16 instruction);

        template <bool stackptr, int reg_dest>
        void Thumb12(u16 instruction);

        template <bool sub>
        void Thumb13(u16 instruction);

        template <bool pop, bool rbit>
        void Thumb14(u16 instruction);

        template <bool load, int reg_base>
        void Thumb15(u16 instruction);

        template <int cond>
        void Thumb16(u16 instruction);

        void Thumb17(u16 instruction);
        void Thumb18(u16 instruction);

        template <bool h>
        void Thumb19(u16 instruction);

        // Command processing
        int Decode(u32 instruction);
        void Execute(u32 instruction, int type);

        // HLE-emulation
        void SWI(int number);
    public:
        // CPU-cyle counter
        int cycles {0};

        // Constructors
        void Init(bool use_bios);

        // Execution functions
        void Step();
        void RaiseIRQ();
    };
}

#include "arm.inl"
