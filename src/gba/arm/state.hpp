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

#include <cstring>

namespace GBA
{
    enum cpu_mode
    {
        MODE_USR = 0x10,
        MODE_FIQ = 0x11,
        MODE_IRQ = 0x12,
        MODE_SVC = 0x13,
        MODE_ABT = 0x17,
        MODE_UND = 0x1B,
        MODE_SYS = 0x1F
    };

    enum status_mask
    {
        MASK_MODE  = 0x1F,
        MASK_THUMB = 0x20,
        MASK_FIQD  = 0x40,
        MASK_IRQD  = 0x80,
        MASK_VFLAG = 0x10000000,
        MASK_CFLAG = 0x20000000,
        MASK_ZFLAG = 0x40000000,
        MASK_NFLAG = 0x80000000
    };

    enum exception_vector
    {
        EXCPT_RESET     = 0x00,
        EXCPT_UNDEFINED = 0x04,
        EXCPT_SWI       = 0x08,
        EXCPT_PREFETCH_ABORT = 0x0C,
        EXCPT_DATA_ABORT     = 0x10,
        EXCPT_INTERRUPT      = 0x18,
        EXCPT_FAST_INTERRUPT = 0x1C
    };

    enum saved_status_register
    {
        SPSR_FIQ = 0,
        SPSR_SVC = 1,
        SPSR_ABT = 2,
        SPSR_IRQ = 3,
        SPSR_UND = 4,
        SPSR_DEF = 5,
        SPSR_COUNT = 6
    };

    struct state
    {
        u32 m_reg[16];

        struct
        {
            u32 m_r8;
            u32 m_r9;
            u32 m_r10;
            u32 m_r11;
            u32 m_r12;
            u32 m_r13;
            u32 m_r14;
        } m_usr, m_fiq;

        struct
        {
            u32 m_r13;
            u32 m_r14;
        } m_svc, m_abt, m_irq, m_und;

        u32 m_cpsr;
        u32 m_spsr[SPSR_COUNT];
        u32* m_spsr_ptr; // pointer to current SPSR.

        state() { reset(); }

        void reset()
        {
            std::memset(this, 0, sizeof(state));
            m_cpsr = MODE_SYS;
            m_spsr_ptr = &m_spsr[SPSR_DEF];
        }
    };
}
