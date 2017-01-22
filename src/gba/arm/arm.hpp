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

#include "enums.hpp"
#include "util/integer.hpp"
#define ARMIGO_INCLUDE

namespace armigo
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

#ifndef ARMIGO_NO_VIRTUAL

        // memory bus methods
        virtual u8 bus_read_byte(u32 address) { return 0; }
        virtual u16 bus_read_hword(u32 address) { return 0; }
        virtual u32 bus_read_word(u32 address) { return 0; }
        virtual void bus_write_byte(u32 address, u8 value) {}
        virtual void bus_write_hword(u32 address, u16 value) {}
        virtual void bus_write_word(u32 address, u32 value) {}

        // swi #nn HLE-handler
        virtual void software_interrupt(int number) {}

#else

        // memory bus methods
        u8 bus_read_byte(u32 address);
        u16 bus_read_hword(u32 address);
        u32 bus_read_word(u32 address);
        void bus_write_byte(u32 address, u8 value);
        void bus_write_hword(u32 address, u16 value);
        void bus_write_word(u32 address, u32 value);

        // swi #nn HLE-handler
        void software_interrupt(int number);

#endif

    private:
        static cpu_bank mode_to_bank(cpu_mode mode);

        void switch_mode(cpu_mode new_mode);

        // conditional helpers
        #include "flags.hpp"

        // memory access helpers
        #include "memory.hpp"

        // shifting helpers
        #include "shifting.hpp"

        // ARM and THUMB interpreter cores
        #include "arm/arm_emu.hpp"
        #include "thumb/thumb_emu.hpp"
    };
}

#undef ARMIGO_INCLUDE
