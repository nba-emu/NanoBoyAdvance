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

#include "arm_enums.hpp"
#include "util/integer.hpp"
#include "util/log.h"
#include "../memory.h"

namespace GBA
{
    class arm
    {
    public:
        int cycles {0};

        arm();

        void reset();
        void step();
        void raise_irq();

        bool get_hle() { return m_hle; }
        void set_hle(bool hle) { m_hle = hle; }

    protected:
        u32 m_reg[16];

        u32 m_bank[BANK_COUNT][7];

        u32 m_cpsr;
        u32 m_spsr[SPSR_COUNT];
        u32* m_spsr_ptr;

        struct
        {
            u32 m_opcode[3];
            int m_index;
            bool m_needs_flush;
        } m_pipeline;

        bool m_hle;

        virtual void software_interrupt(int number);

    private:
        static cpu_bank mode_to_bank(cpu_mode mode);
        void switch_mode(cpu_mode new_mode);

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

        // methods that emulate thumb instructions
        #include "thumb_emu.hpp"

        // ARM command processing
        int Decode(u32 instruction);
        void Execute(u32 instruction, int type);
    };
}

#include "arm.inl"
