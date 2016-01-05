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
#include <vector>
#include "common/types.h"
#include "common/log.h"
#include "memory.h"
#include "arm7_breakpoint.h"
#include "arm7_callstack.h"

//#define CPU_LOG

// Macro for easier register access
#define reg(r) *gprs[r]

using namespace std;

namespace NanoboyAdvance
{
    class ARM7
    {
        // Grants the processor access to the emulated mmu
        Memory* memory;

        // The ARM7TMDI-S has 31 32-bit general purpose register of
        // which 16 are visible at one time.
        uint r0 {0}, r1 {0}, r2 {0}, r3 {0}, r4 {0}, r5 {0}, r6 {0}, r7 {0}, r8 {0}, r9 {0}, r10 {0}, r11 {0}, r12 {0}, r13 {0}, r14 {0}, r15 {0};
        uint r8_fiq {0}, r9_fiq {0}, r10_fiq {0}, r11_fiq {0}, r12_fiq {0}, r13_fiq {0}, r14_fiq {0};
        uint r13_svc {0}, r14_svc {0};
        uint r13_abt {0}, r14_abt {0};
        uint r13_irq {0}, r14_irq {0};
        uint r13_und {0}, r14_und {0};

        // Mapping array for visible general purpose registers
        uint* gprs[16];

        // Current program status register (contains status flags)
        uint cpsr {System};
        uint spsr_fiq {0}, spsr_svc {0}, spsr_abt {0}, spsr_irq {0}, spsr_und {0}, spsr_def {0};

        // A pointer pointing on the Saved program status register of the current mode
        uint* pspsr {nullptr};

        // In some way emulate the processor's pipeline
        uint pipe_opcode[3];
        int pipe_decode[3];
        int pipe_status {0};
        bool flush_pipe {false};

        // Some games seem to read from bios
        uint last_fetched_bios {0};

        // Indicates wether interrupts and swi should be processed using
        // the bios or using a hle attempt
        bool hle;

        // Maps the visible registers (according to cpsr) to gprs
        void RemapRegisters();

        // Condition code altering methods
        inline void CalculateSign(uint result)
        {
            cpsr = result & 0x80000000 ? (cpsr | SignFlag) : (cpsr & ~SignFlag);
        }

        inline void CalculateZero(ulong result)
        {
            cpsr = result == 0 ? (cpsr | ZeroFlag) : (cpsr & ~ZeroFlag);
        }

        inline void AssertCarry(bool carry)
        {
            cpsr = carry ? (cpsr | CarryFlag) : (cpsr & ~CarryFlag);
        }

        inline void CalculateOverflowAdd(uint result, uint operand1, uint operand2)
        {
            bool overflow = ((operand1) >> 31 == (operand2) >> 31) && ((result) >> 31 != (operand2) >> 31);
            cpsr = overflow ? (cpsr | OverflowFlag) : (cpsr & ~OverflowFlag);
        }

        inline void CalculateOverflowSub(uint result, uint operand1, uint operand2)
        {
            bool overflow = ((operand1) >> 31 != (operand2) >> 31) && ((result) >> 31 == (operand2) >> 31);
            cpsr = overflow ? (cpsr | OverflowFlag) : (cpsr & ~OverflowFlag);
        }

        // Shifter methods
        void LSL(uint& operand, uint amount, bool& carry);
        void LSR(uint& operand, uint amount, bool& carry, bool immediate);
        void ASR(uint& operand, uint amount, bool& carry, bool immediate);
        void ROR(uint& operand, uint amount, bool& carry, bool immediate);

        // Memory methods
        ubyte ReadByte(uint offset);
        ushort ReadHWord(uint offset);
        uint ReadWord(uint offset);
        uint ReadWordRotated(uint offset);
        void WriteByte(uint offset, ubyte value);
        void WriteHWord(uint offset, ushort value);
        void WriteWord(uint offset, uint value);

        // Command processing
        int ARMDecode(uint instruction);
        void ARMExecute(uint instruction, int type);
        int THUMBDecode(ushort instruction);
        void THUMBExecute(ushort instruction, int type);

        // Debugging
        void TriggerMemoryBreakpoint(bool write, uint address, int size);
        void TriggerSVCBreakpoint(uint bios_call);

        // Used to emulate software interrupts
        void SWI(int number);
    public:
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
        enum CrashReason
        {
            BadPC,
            BadMemory
        };

        // Debugging
        vector<ARM7Breakpoint*> breakpoints;
        ARM7Breakpoint* last_breakpoint {nullptr};
        bool hit_breakpoint {false};
        bool crashed {false};
        CrashReason crash_reason;

        // Constructor
        ARM7(Memory* memory, bool use_bios);

        // Schedule pipeline
        void Step();

        // Trigger IRQ exception
        void FireIRQ();

        // Register getter / setters
        uint GetGeneralRegister(int number);
        void SetGeneralRegister(int number, uint value);
        uint GetStatusRegister();
        uint GetSavedStatusRegister();
        uint GetStackPointerMode(int mode);
    };
}
