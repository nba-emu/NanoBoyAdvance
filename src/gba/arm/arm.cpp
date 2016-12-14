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

#include "arm.hpp"

namespace GBA
{
    void arm::Init(bool hle)
    {
        m_Pipe.m_Index = 0;

        // Skip bios boot logo
        m_state.m_reg[15] = 0x8000000;
        m_state.m_reg[13] = 0x3007F00;
        m_state.m_bank[BANK_SVC][BANK_R13] = 0x3007FE0;
        m_state.m_bank[BANK_IRQ][BANK_R13] = 0x3007FA0;
        ////LoadRegisters();
        RefillPipeline();

        // Set hle flag
        this->hle = hle;
    }

    void arm::Step()
    {
        bool thumb = m_state.m_cpsr & MASK_THUMB;

        // Forcibly align r15
        m_state.m_reg[15] &= thumb ? ~1 : ~3;

        // Dispatch instruction loading and execution
        if (thumb)
        {
            if (m_Pipe.m_Index == 0)
                m_Pipe.m_Opcode[2] = Memory::ReadHWord(m_state.m_reg[15]);
            else
                m_Pipe.m_Opcode[m_Pipe.m_Index - 1] = Memory::ReadHWord(m_state.m_reg[15]);

            // Execute the thumb instruction via a method lut.
            u16 instruction = m_Pipe.m_Opcode[m_Pipe.m_Index];
            (this->*THUMB_TABLE[instruction >> 6])(instruction);
        }
        else
        {
            if (m_Pipe.m_Index == 0)
                m_Pipe.m_Opcode[2] = Memory::ReadWord(m_state.m_reg[15]);
            else
                m_Pipe.m_Opcode[m_Pipe.m_Index - 1] = Memory::ReadWord(m_state.m_reg[15]);

            Execute(m_Pipe.m_Opcode[m_Pipe.m_Index], Decode(m_Pipe.m_Opcode[m_Pipe.m_Index]));
        }

        if (m_Pipe.m_Flush)
        {
            m_Pipe.m_Index = 0;
            m_Pipe.m_Flush = false;
            RefillPipeline();
            return;
        }

        // Update pipeline status
        m_Pipe.m_Index = (m_Pipe.m_Index + 1) % 3;

        // Update instruction pointer
        m_state.m_reg[15] += thumb ? 2 : 4;
    }

    void arm::RaiseIRQ()
    {
        if ((m_state.m_cpsr & MASK_IRQD) == 0)
        {
            bool thumb = m_state.m_cpsr & MASK_THUMB;

            // "Useless" pipeline prefetch
            cycles += Memory::SequentialAccess(m_state.m_reg[15], thumb ? ACCESS_HWORD : ACCESS_WORD);

            // Store return address in r14<irq>
            m_state.m_bank[BANK_IRQ][BANK_R14] = m_state.m_reg[15] - (thumb ? 4 : 8) + 4;

            // Save program status and switch mode
            m_state.m_spsr[SPSR_IRQ] = m_state.m_cpsr;
            m_state.switch_mode(MODE_IRQ);
            m_state.m_cpsr = (m_state.m_cpsr & ~MASK_THUMB) | MASK_IRQD;
            ////SaveRegisters();
            ////m_state.m_cpsr = (m_state.m_cpsr & ~(MASK_MODE | MASK_THUMB)) | MODE_IRQ | MASK_IRQD;
            ////LoadRegisters();

            // Jump to exception vector
            m_state.m_reg[15] = EXCPT_INTERRUPT;
            m_Pipe.m_Index = 0;
            m_Pipe.m_Flush = false;
            RefillPipeline();

            // Emulate pipeline refill timings
            cycles += Memory::NonSequentialAccess(m_state.m_reg[15], ACCESS_WORD) +
                      Memory::SequentialAccess(m_state.m_reg[15] + 4, ACCESS_WORD);
        }
    }

    void arm::SWI(int number)
    {
        switch (number)
        {
        case 0x06:
        {
            if (m_state.m_reg[1] != 0)
            {
                u32 mod = m_state.m_reg[0] % m_state.m_reg[1];
                u32 div = m_state.m_reg[0] / m_state.m_reg[1];
                m_state.m_reg[0] = div;
                m_state.m_reg[1] = mod;
            } else
            {
                LOG(LOG_ERROR, "SWI6h: Attempted division by zero.");
            }
            break;
        }
        case 0x05:
            m_state.m_reg[0] = 1;
            m_state.m_reg[1] = 1;
        case 0x04:
            Interrupt::WriteMasterEnableLow(1);
            Interrupt::WriteMasterEnableHigh(0);

            // If r0 is one IF must be cleared
            if (m_state.m_reg[0] == 1)
            {
                Interrupt::WriteInterruptFlagLow(0);
                Interrupt::WriteInterruptFlagHigh(0);
            }

            // Sets GBA into halt state, waiting for specific interrupt(s) to occur.
            Memory::m_IntrWait = true;
            Memory::m_IntrWaitMask = m_state.m_reg[1];
            Memory::m_HaltState = HALTSTATE_HALT;
            break;
        case 0x0B:
        {
            u32 source = m_state.m_reg[0];
            u32 dest = m_state.m_reg[1];
            u32 length = m_state.m_reg[2] & 0xFFFFF;
            bool fixed = m_state.m_reg[2] & (1 << 24) ? true : false;

            if (m_state.m_reg[2] & (1 << 26))
            {
                for (u32 i = 0; i < length; i++)
                {
                    WriteWord(dest, ReadWord(source));
                    dest += 4;

                    if (!fixed) source += 4;
                }
            }
            else
            {
                for (u32 i = 0; i < length; i++)
                {
                    WriteHWord(dest, ReadHWord(source));
                    dest += 2;

                    if (!fixed) source += 2;
                }
            }
            break;
        }
        case 0x0C:
        {
            u32 source = m_state.m_reg[0];
            u32 dest = m_state.m_reg[1];
            u32 length = m_state.m_reg[2] & 0xFFFFF;
            bool fixed = m_state.m_reg[2] & (1 << 24) ? true : false;

            for (u32 i = 0; i < length; i++)
            {
                WriteWord(dest, ReadWord(source));
                dest += 4;
                if (!fixed) source += 4;
            }
            break;
        }
        case 0x11:
        case 0x12:
        {
            int amount = Memory::ReadWord(m_state.m_reg[0]) >> 8;
            u32 source = m_state.m_reg[0] + 4;
            u32 dest = m_state.m_reg[1];

            while (amount > 0)
            {
                u8 encoder = Memory::ReadByte(source++);

                // Process 8 blocks encoded by the encoder
                for (int i = 7; i >= 0; i--)
                {
                    if (encoder & (1 << i))
                    {
                        // Compressed
                        u16 value = Memory::ReadHWord(source);
                        u32 disp = (value >> 8) | ((value & 0xF) << 8);
                        u32 n = ((value >> 4) & 0xF) + 3;
                        source += 2;

                        for (u32 j = 0; j < n; j++)
                        {
                            u16 value = Memory::ReadByte(dest - disp - 1);
                            Memory::WriteHWord(dest, value);
                            dest++;

                            if (--amount == 0) return;
                        }
                    }
                    else
                    {
                        // Uncompressed
                        u8 value = Memory::ReadByte(source++);
                        Memory::WriteHWord(dest++, value);

                        if (--amount == 0) return;
                    }
                }
            }
            break;
        }
        default:
            #ifdef DEBUG
            LOG(LOG_ERROR, "Unimplemented software interrupt 0x%x", number);
            #endif
            break;
        }
    }
}
