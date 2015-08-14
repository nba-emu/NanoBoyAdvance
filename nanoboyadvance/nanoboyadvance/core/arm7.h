/*
* Copyright (C) 2015 Frederic Meyer
*
* This file is part of nanoboyadvance.
*
* nanoboyadvance is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* nanoboyadvance is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <iostream>
#include <sstream>
#include "common/types.h"
#include "common/log.h"
#include "memory.h"

//#define CPU_LOG

// Macro for easier register access
#define reg(r) *gprs[r]

// TODO: We can create inlined methods for these
// Alters the sign flag depending on the MSB of the value being set or not
#define calculate_sign(n) {\
    if ((n) & 0x80000000)\
    {\
        cpsr |= SignFlag;\
    }\
    else\
    {\
        cpsr &= ~SignFlag;\
    }\
}

// Alters the zero flag depending on the value being zero or not
#define calculate_zero(n) {\
    if ((n) == 0)\
    {\
        cpsr |= ZeroFlag;\
    }\
    else\
    {\
        cpsr &= ~ZeroFlag;\
    }\
}

// Sets or unsets the carry flag depending on the given condition being true or false
#define assert_carry(n) {\
    if ((n))\
    {\
        cpsr |= CarryFlag;\
    }\
    else\
    {\
        cpsr &= ~CarryFlag;\
    }\
}

// Alters the overflow flag depending on the result and the operands of an addition
#define calculate_overflow_add(result, operand1, operand2) {\
    if (((operand1) >> 31 == (operand2) >> 31) && ((result) >> 31 != (operand2) >> 31))\
    {\
        cpsr |= OverflowFlag;\
    }\
    else\
    {\
        cpsr &= ~OverflowFlag;\
    }\
}

// Alters the overflow flag depending on the result and the operands of a subtraction
#define calculate_overflow_sub(result, operand1, operand2) {\
    if (((operand1) >> 31 != (operand2) >> 31) && ((result) >> 31 == (operand2) >> 31))\
    {\
        cpsr |= OverflowFlag;\
    }\
    else\
    {\
        cpsr &= ~OverflowFlag;\
    }\
}

using namespace std;

namespace NanoboyAdvance
{
    class ARM7
    {
        // Grants the processor access to the emulated mmu which
        // organizes the memory in pages (bit 24-31 determine the page number)
        PagedMemory* memory;
        
        // The ARM7TMDI-S has 31 32-bit general purpose register of
        // which 16 are visible at one time.
        uint r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15;
        uint r8_fiq, r9_fiq, r10_fiq, r11_fiq, r12_fiq, r13_fiq, r14_fiq;
        uint r13_svc, r14_svc;
        uint r13_abt, r14_abt;
        uint r13_irq, r14_irq;
        uint r13_und, r14_und;
        
        // Mapping array for visible general purpose registers
        uint* gprs[16];
        
        // Current program status register (contains status flags)
        uint cpsr;
        uint spsr_fiq, spsr_svc, spsr_abt, spsr_irq, spsr_und, spsr_def;
        // A pointer pointing on the Saved program status register of the current mode
        uint* pspsr;
        
        // In some way emulate the processor's pipeline
        uint pipe_opcode[3];
        int pipe_decode[3];
        int pipe_status;
        bool flush_pipe;

        inline void RemapRegisters();
        int ARMDecode(uint instruction);
        void ARMExecute(uint instruction, int type);
        int THUMBDecode(ushort instruction);
        void THUMBExecute(ushort instruction, int type);
    public:
        // TODO: Maybe use a traditional define?
        enum ARM7Mode
        {
            User = 0x10,
            FIQ = 0x11,
            IRQ = 0x12,
            SVC = 0x13,
            Abort = 0x17,
            Undefined = 0x1B,
            System = 0x1F
        };
        enum CPSRFlags
        {
            Thumb = 0x20,
            FIQDisable = 0x40,
            IRQDisable = 0x80,
            OverflowFlag = 0x10000000,
            CarryFlag = 0x20000000,
            ZeroFlag = 0x40000000,
            SignFlag = 0x80000000
        };
        ARM7(PagedMemory* memory);
        void Step();
        string ARMDisasm(uint base, uint instruction);
    };
}