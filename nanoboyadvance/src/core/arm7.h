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
#include "gba_memory.h"

#define ARM7_FASTHAX
#define arm_pack_instr(i) ((i) & 0xFFF) | (((i) & 0x0FF00000) >> 8)

// Macro for easier register access
#define reg(r) *gprs[r]

static const int ARM_CALLBACK_EXECUTE = 0;
static const int ARM_CALLBACK_SWI = 1;
static const int ARM_CALLBACK_IRQ = 2;
static const int ARM_CALLBACK_SWI_RET = 3;
static const int ARM_CALLBACK_IRQ_RET = 4;
static const int ARM_CALLBACK_CALL = 5;
static const int ARM_CALLBACK_RET = 6;

typedef struct 
{
    u32 address;
    bool thumb;
} ARMCallbackExecute;

typedef void (*ARMCallback)(int reason, void* data);

using namespace std;

namespace NanoboyAdvance
{
    class ARM7
    {
        // Grants the processor access to the emulated mmu
        GBAMemory* memory;

        // The ARM7TMDI-S has 31 32-bit general purpose register of
        // which 16 are visible at one time.
        u32 r0 {0}, r1 {0}, r2 {0}, r3 {0}, r4 {0}, r5 {0}, r6 {0}, r7 {0}, r8 {0}, r9 {0}, r10 {0}, r11 {0}, r12 {0}, r13 {0}, r14 {0}, r15 {0};
        u32 r8_fiq {0}, r9_fiq {0}, r10_fiq {0}, r11_fiq {0}, r12_fiq {0}, r13_fiq {0}, r14_fiq {0};
        u32 r13_svc {0}, r14_svc {0};
        u32 r13_abt {0}, r14_abt {0};
        u32 r13_irq {0}, r14_irq {0};
        u32 r13_und {0}, r14_und {0};

        // Mapping array for visible general purpose registers
        u32* gprs[16];

        // Current program status register (contains status flags)
        u32 cpsr { (u32)ARM7Mode::System};
        u32 spsr_fiq {0}, spsr_svc {0}, spsr_abt {0}, spsr_irq {0}, spsr_und {0}, spsr_def {0};

        // A pointer pointing on the Saved program status register of the current mode
        u32* pspsr {nullptr};

        #ifdef ARM7_FASTHAX
        int thumb_decode[0x10000];
        int arm_decode[0x100000];
        #endif

        // In some way emulate the processor's pipeline
        u32 pipe_opcode[3];
        int pipe_decode[3];
        int pipe_status {0};
        bool flush_pipe {false};

        // Emulate "unpredictable" behaviour
        u32 last_fetched_opcode {0};
        u32 last_fetched_offset {0};
        u32 last_bios_offset {0};

        // Gets called on certain events like instruction execution, swi, etc.
        // Do not ever call this directly! Use safe DebugHook instead!
        ARMCallback debug_hook { NULL };
        
        // Indicates wether interrupts and swi should be processed using
        // the bios or using a hle attempt
        bool hle;

        // Maps the visible registers (according to cpsr) to gprs
        void RemapRegisters();
        
        // Pointer-safe call to debug_hook (avoid nullpointer)
        inline void DebugHook(int reason, void* data)
        {
            if (debug_hook != NULL)
            {
                debug_hook(reason, data);
            }
        }

        // Condition code altering methods
        inline void CalculateSign(u32 result)
        {
            cpsr = result & 0x80000000 ? (cpsr | SignFlag) : (cpsr & ~SignFlag);
        }

        inline void CalculateZero(u64 result)
        {
            cpsr = result == 0 ? (cpsr | ZeroFlag) : (cpsr & ~ZeroFlag);
        }

        inline void AssertCarry(bool carry)
        {
            cpsr = carry ? (cpsr | CarryFlag) : (cpsr & ~CarryFlag);
        }

        inline void CalculateOverflowAdd(u32 result, u32 operand1, u32 operand2)
        {
            bool overflow = ((operand1) >> 31 == (operand2) >> 31) && ((result) >> 31 != (operand2) >> 31);
            cpsr = overflow ? (cpsr | OverflowFlag) : (cpsr & ~OverflowFlag);
        }

        inline void CalculateOverflowSub(u32 result, u32 operand1, u32 operand2)
        {
            bool overflow = ((operand1) >> 31 != (operand2) >> 31) && ((result) >> 31 == (operand2) >> 31);
            cpsr = overflow ? (cpsr | OverflowFlag) : (cpsr & ~OverflowFlag);
        }

        // Shifter methods
        void LSL(u32& operand, u32 amount, bool& carry);
        void LSR(u32& operand, u32 amount, bool& carry, bool immediate);
        void ASR(u32& operand, u32 amount, bool& carry, bool immediate);
        void ROR(u32& operand, u32 amount, bool& carry, bool immediate);

        // Memory methods
        u8 ReadByte(u32 offset);
        u16 ReadHWord(u32 offset);
        u32 ReadHWordSigned(u32 offset);
        u32 ReadWord(u32 offset);
        u32 ReadWordRotated(u32 offset);
        void WriteByte(u32 offset, u8 value);
        void WriteHWord(u32 offset, u16 value);
        void WriteWord(u32 offset, u32 value);

        // Command processing
        int ARMDecode(u32 instruction);
        void ARMExecute(u32 instruction, int type);
        int THUMBDecode(u16 instruction);
        void THUMBExecute(u16 instruction, int type);

        // Used to emulate software interrupts
        void SWI(int number);
    public:
        enum class ARM7Mode
        {
            User = 0x10,
            FIQ = 0x11,
            IRQ = 0x12,
            SVC = 0x13,
            Abort = 0x17,
            Undefined = 0x1B,
            System = 0x1F
        };
        enum CPSRFlags // TODO: enum class
        {
            Thumb = 0x20,
            FIQDisable = 0x40,
            IRQDisable = 0x80,
            OverflowFlag = 0x10000000,
            CarryFlag = 0x20000000,
            ZeroFlag = 0x40000000,
            SignFlag = 0x80000000
        };

        // Constructor
        ARM7(GBAMemory* memory, bool use_bios);
        
        // Execution functions
        void Step(); // schedule pipeline
        void FireIRQ(); // enter bios irq handler

        // Debugging
        u32 GetGeneralRegister(ARM7Mode mode, int r);
        u32 GetCurrentStatusRegister();
        u32 GetSavedStatusRegister(ARM7Mode mode);  
        void SetCallback(ARMCallback hook);
        void SetGeneralRegister(ARM7Mode mode, int r, u32 value);
        void SetCurrentStatusRegister(u32 value);
        void SetSavedStatusRegister(ARM7Mode mode, u32 value);     
    };
}
