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


#include "util/types.h"
#include "util/log.h"
#include "memory.h"


namespace NanoboyAdvance
{
    class ARM7
    {
        GBAMemory* memory;
        
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
        
        inline void CalculateSign(u32 result)
            { cpsr = result & 0x80000000 ? (cpsr | SignFlag) : (cpsr & ~SignFlag); }

        inline void CalculateZero(u64 result)
            { cpsr = result == 0 ? (cpsr | ZeroFlag) : (cpsr & ~ZeroFlag); }

        inline void AssertCarry(bool carry)
            { cpsr = carry ? (cpsr | CarryFlag) : (cpsr & ~CarryFlag); }

        inline void CalculateOverflowAdd(u32 result, u32 operand1, u32 operand2)
        {
            bool overflow = !(((operand1) ^ (operand2)) >> 31) && ((result) ^ (operand2)) >> 31;
            cpsr = overflow ? (cpsr | OverflowFlag) : (cpsr & ~OverflowFlag);
        }

        inline void CalculateOverflowSub(u32 result, u32 operand1, u32 operand2)
        {
            bool overflow = ((operand1) ^ (operand2)) >> 31 && !(((result) ^ (operand2)) >> 31);
            cpsr = overflow ? (cpsr | OverflowFlag) : (cpsr & ~OverflowFlag);
        }

        static inline void LSL(u32& operand, u32 amount, bool& carry)
        {
            if (amount == 0) return;

            for (u32 i = 0; i < amount; i++)
            {
                carry = operand & 0x80000000 ? true : false;
                operand <<= 1;
            }
        }

        static inline void LSR(u32& operand, u32 amount, bool& carry, bool immediate)
        {
            // LSR #0 equals to LSR #32
            amount = immediate & (amount == 0) ? 32 : amount;

            for (u32 i = 0; i < amount; i++)
            {
                carry = operand & 1 ? true : false;
                operand >>= 1;
            }
        }

        static inline void ASR(u32& operand, u32 amount, bool& carry, bool immediate)
        {
            u32 sign_bit = operand & 0x80000000;

            // ASR #0 equals to ASR #32
            amount = (immediate && (amount == 0)) ? 32 : amount;

            for (u32 i = 0; i < amount; i++)
            {
                carry = operand & 1 ? true : false;
                operand = (operand >> 1) | sign_bit;
            }
        }

        static inline void ROR(u32& operand, u32 amount, bool& carry, bool immediate)
        {
            // ROR #0 equals to RRX #1
            if (amount != 0 || !immediate)
            {
                for (u32 i = 1; i <= amount; i++)
                {
                    u32 high_bit = (operand & 1) ? 0x80000000 : 0;
                    operand = (operand >> 1) | high_bit;
                    carry = high_bit == 0x80000000;
                }
            }
            else
            {
                bool old_carry = carry;
                carry = (operand & 1) ? true : false;
                operand = (operand >> 1) | (old_carry ? 0x80000000 : 0);
            }
        }

        inline u8 ReadByte(u32 offset)
            { return memory->ReadByte(offset); }

        inline u32 ReadHWord(u32 offset)
        {
            if (offset & 1)
            {
                u32 value = memory->ReadHWord(offset & ~1);
                return (value >> 8) | (value << 24); 
            }        
            return memory->ReadHWord(offset);
        }

        inline u32 ReadHWordSigned(u32 offset)
        {
            u32 value = 0;        
            if (offset & 1) 
            {
                value = memory->ReadByte(offset);
                if (value & 0x80) 
                    value |= 0xFFFFFF00;
            }
            else 
            {
                value = memory->ReadHWord(offset);
                if (value & 0x8000)
                    value |= 0xFFFF0000;
            }
            return value;
        }

        inline u32 ReadWord(u32 offset)
            { return memory->ReadWord(offset & ~3); }

        inline u32 ReadWordRotated(u32 offset)
        {
            u32 value = ReadWord(offset & ~3);
            int amount = (offset & 3) * 8;
            return amount == 0 ? value : ((value >> amount) | (value << (32 - amount)));
        }

        inline void WriteByte(u32 offset, u8 value)
            { memory->WriteByte(offset, value); }

        inline void WriteHWord(u32 offset, u16 value)
        {
            offset &= ~1;
            memory->WriteHWord(offset, value);
        }

        inline void WriteWord(u32 offset, u32 value)
        {
            offset &= ~3;
            memory->WriteWord(offset, value);
        }

        inline void RefillPipeline()
        {
            if (cpsr & ThumbFlag)
            {
                pipe.opcode[0] = memory->ReadHWord(r[15]);
                pipe.opcode[1] = memory->ReadHWord(r[15] + 2);
                r[15] += 4;
            }
            else
            {
                pipe.opcode[0] = memory->ReadWord(r[15]);
                pipe.opcode[1] = memory->ReadWord(r[15] + 4);
                r[15] += 8;
            }
        }

        // Saves/loads registers of current mode
        void SaveRegisters();
        void LoadRegisters();

        // Command processing
        int Decode(u32 instruction);
        void Execute(u32 instruction, int type);
        void ExecuteThumb(u16 instruction);

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

        // Constructor
        ARM7(GBAMemory* memory, bool use_bios);
        
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
