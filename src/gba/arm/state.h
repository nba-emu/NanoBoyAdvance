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


#ifndef __NBA_STATE_H__
#define __NBA_STATE_H__


#include <cstring>


namespace GBA
{
    enum Mode
    {
        MODE_USR = 0x10,
        MODE_FIQ = 0x11,
        MODE_IRQ = 0x12,
        MODE_SVC = 0x13,
        MODE_ABT = 0x17,
        MODE_UND = 0x1B,
        MODE_SYS = 0x1F
    };

    enum StatusMask
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

    enum ExceptionVector
    {
        EXCPT_RESET     = 0x00,
        EXCPT_UNDEFINED = 0x04,
        EXCPT_SWI       = 0x08,
        EXCPT_PREFETCH_ABORT = 0x0C,
        EXCPT_DATA_ABORT     = 0x10,
        EXCPT_INTERRUPT      = 0x18,
        EXCPT_FAST_INTERRUPT = 0x1C
    };

    enum SPSR
    {
        SPSR_FIQ = 0,
        SPSR_SVC = 1,
        SPSR_ABT = 2,
        SPSR_IRQ = 3,
        SPSR_UND = 4,
        SPSR_DEF = 5,
        SPSR_COUNT = 6
    };

    struct ARMState
    {
        u32 m_R[16];

        struct
        {
            u32 m_R8;
            u32 m_R9;
            u32 m_R10;
            u32 m_R11;
            u32 m_R12;
            u32 m_R13;
            u32 m_R14;
        } m_USR, m_FIQ;

        struct
        {
            u32 m_R13;
            u32 m_R14;
        } m_SVC, m_ABT, m_IRQ, m_UND;

        u32 m_CPSR;
        u32 m_SPSR[SPSR_COUNT];
        u32* m_PSPSR; // pointer to current SPSR.

        ARMState() { Reset(); }

        void Reset()
        {
            std::memset(this, 0, sizeof(ARMState));
            m_CPSR = MODE_SYS;
            m_PSPSR = &m_SPSR[SPSR_DEF];
        }
    };
}


#endif // __NBA_STATE_H__
