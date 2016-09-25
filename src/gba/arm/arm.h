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


#include "common/types.h"
#include "common/log.h"
#include "../memory.h"


namespace GBA
{
    class ARM7
    {
        typedef void (ARM7::*ThumbInstruction)(u16);
        static const ThumbInstruction thumb_table[1024];
        
        // General Purpose Registers
        u32 r[16] {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        u32 r8_usr {0}, r8_fiq {0};
        u32 r9_usr {0}, r9_fiq {0};
        u32 r10_usr {0}, r10_fiq {0};
        u32 r11_usr {0}, r11_fiq {0};
        u32 r12_usr {0}, r12_fiq {0};
        u32 r13_usr {0}, r13_fiq {0}, r13_svc {0}, r13_abt {0}, r13_irq {0}, r13_und {0};
        u32 r14_usr {0}, r14_fiq {0}, r14_svc {0}, r14_abt {0}, r14_irq {0}, r14_und {0};

        // Artifact of old code. Remove when I got time.
        #define reg(i) this->r[i]

        // Program Status Registers
        u32 cpsr { (u32)Mode::SYS};
        u32 spsr_fiq {0};
        u32 spsr_svc {0};
        u32 spsr_abt {0};
        u32 spsr_irq {0};
        u32 spsr_und {0};
        u32 spsr_def {0};

        // Points to current SPSR
        u32* pspsr {nullptr};

        // Stores pipeline state
        struct Pipeline
        {
            u32 opcode[3];
            int status {0};
            bool flush {false};
        } pipe;
        
        bool hle;
        
        void CalculateSign(u32 result);
        void CalculateZero(u64 result);
        void AssertCarry(bool carry);
        void CalculateOverflowAdd(u32 result, u32 operand1, u32 operand2);
        void CalculateOverflowSub(u32 result, u32 operand1, u32 operand2);
        static void LSL(u32& operand, u32 amount, bool& carry);
        static void LSR(u32& operand, u32 amount, bool& carry, bool immediate);
        static void ASR(u32& operand, u32 amount, bool& carry, bool immediate);
        static void ROR(u32& operand, u32 amount, bool& carry, bool immediate);

        u8 ReadByte(u32 offset);
        u32 ReadHWord(u32 offset);
        u32 ReadHWordSigned(u32 offset);
        u32 ReadWord(u32 offset);
        u32 ReadWordRotated(u32 offset);
        void WriteByte(u32 offset, u8 value);
        void WriteHWord(u32 offset, u16 value);
        void WriteWord(u32 offset, u32 value);
        void RefillPipeline();

        // Saves/loads registers of current mode
        void SaveRegisters();
        void LoadRegisters();

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

        inline void ExecuteThumb(u16 instruction)
        {
            (this->*thumb_table[instruction >> 6])(instruction);
        }

        // HLE-emulation
        void SWI(int number);
    public:
        enum class Mode
        {
            USR = 0x10,
            FIQ = 0x11,
            IRQ = 0x12,
            SVC = 0x13,
            ABT = 0x17,
            UND = 0x1B,
            SYS = 0x1F
        };

        enum PSRMask
        {
            ModeField = 0x1F,
            ThumbFlag = 0x20,
            FiqDisable = 0x40,
            IrqDisable = 0x80,
            OverflowFlag = 0x10000000,
            CarryFlag = 0x20000000,
            ZeroFlag = 0x40000000,
            SignFlag = 0x80000000
        };

        enum class Exception
        {
            Reset = 0x00,
            UndefinedInstruction = 0x04,
            SoftwareInterrupt = 0x08,
            PrefetchAbort = 0x0C,
            DataAbort = 0x10,
            Interrupt = 0x18,
            FastInterrupt = 0x1C
        };

        // CPU-cyle counter
        int cycles {0};

        // Constructors
        void Init(bool use_bios);
        
        // Execution functions
        void Step();
        void RaiseIRQ();

        // Register read and write access methods
        u32 GetGeneralRegister(Mode mode, int r);
        u32 GetCurrentStatusRegister();
        u32 GetSavedStatusRegister(Mode mode);  
        void SetGeneralRegister(Mode mode, int r, u32 value);
        void SetCurrentStatusRegister(u32 value);
        void SetSavedStatusRegister(Mode mode, u32 value);     
    };
}

#include "arm.inl"
