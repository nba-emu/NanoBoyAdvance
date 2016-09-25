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


#include "arm.h"


namespace GBA
{
    void ARM7::Init(bool hle)
    {
        m_Pipe.m_Index = 0;

        // Skip bios boot logo
        m_State.m_R[15] = 0x8000000;
        m_State.m_USR.m_R13 = 0x3007F00;
        m_State.m_SVC.m_R13 = 0x3007FE0;
        m_State.m_IRQ.m_R13 = 0x3007FA0;
        LoadRegisters();
        RefillPipeline();

        // Set hle flag
        this->hle = hle;
    }
    
    u32 ARM7::GetGeneralRegister(Mode mode, int r)
    {
        u32 value;
        Mode old_mode = (Mode)(m_State.m_CPSR & MASK_MODE);

        // Temporary switch to requested mode
        SaveRegisters();
        m_State.m_CPSR = (m_State.m_CPSR & ~MASK_MODE) | (u32)mode;
        LoadRegisters();

        // Get value and switch back to correct mode
        value = reg(r);
        m_State.m_CPSR = (m_State.m_CPSR & ~MASK_MODE) | (u32)old_mode;
        LoadRegisters();

        return value;
    }

    u32 ARM7::GetCurrentStatusRegister()
    {
        return m_State.m_CPSR;
    }

    u32 ARM7::GetSavedStatusRegister(Mode mode)
    {
        switch (mode)
        {
        case MODE_FIQ: return m_State.m_SPSR[SPSR_FIQ];
        case MODE_SVC: return m_State.m_SPSR[SPSR_SVC];
        case MODE_ABT: return m_State.m_SPSR[SPSR_ABT];
        case MODE_IRQ: return m_State.m_SPSR[SPSR_IRQ];
        case MODE_UND: return m_State.m_SPSR[SPSR_UND];
        }
        return 0;
    }

    void ARM7::SetGeneralRegister(Mode mode, int r, u32 value)
    {
        Mode old_mode = (Mode)(m_State.m_CPSR & MASK_MODE);

        // Temporary switch to requested mode
        SaveRegisters();
        m_State.m_CPSR = (m_State.m_CPSR & ~MASK_MODE) | (u32)mode;
        LoadRegisters();

        // Write register and switch back
        reg(r) = value;
        SaveRegisters();
        m_State.m_CPSR = (m_State.m_CPSR & ~MASK_MODE) | (u32)old_mode;
        LoadRegisters();
    }

    void ARM7::SetCurrentStatusRegister(u32 value)
    {
        m_State.m_CPSR = value;
    }
    
    void ARM7::SetSavedStatusRegister(Mode mode, u32 value)
    {
        switch (mode)
        {
        case MODE_FIQ: m_State.m_SPSR[SPSR_FIQ] = value; break;
        case MODE_SVC: m_State.m_SPSR[SPSR_SVC] = value; break;
        case MODE_ABT: m_State.m_SPSR[SPSR_ABT] = value; break;
        case MODE_IRQ: m_State.m_SPSR[SPSR_IRQ] = value; break;
        case MODE_UND: m_State.m_SPSR[SPSR_UND] = value; break;
        }
    }

    void ARM7::Step()
    {
        bool thumb = m_State.m_CPSR & MASK_THUMB;

        // Forcibly align r15
        m_State.m_R[15] &= thumb ? ~1 : ~3;

        // Dispatch instruction loading and execution
        if (thumb)
        {
            if (m_Pipe.m_Index == 0)
                m_Pipe.m_Opcode[2] = Memory::ReadHWord(m_State.m_R[15]);
            else
                m_Pipe.m_Opcode[m_Pipe.m_Index - 1] = Memory::ReadHWord(m_State.m_R[15]);
            ExecuteThumb(m_Pipe.m_Opcode[m_Pipe.m_Index]);
        }
        else
        {
            if (m_Pipe.m_Index == 0)
                m_Pipe.m_Opcode[2] = Memory::ReadWord(m_State.m_R[15]);
            else
                m_Pipe.m_Opcode[m_Pipe.m_Index - 1] = Memory::ReadWord(m_State.m_R[15]);
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
        m_State.m_R[15] += thumb ? 2 : 4;
    }

    void ARM7::SaveRegisters()
    {
        switch (m_State.m_CPSR & MASK_MODE)
        {
        case MODE_USR:
        case MODE_SYS:
            m_State.m_USR.m_R8 = m_State.m_R[8];
            m_State.m_USR.m_R9 = m_State.m_R[9];
            m_State.m_USR.m_R10 = m_State.m_R[10];
            m_State.m_USR.m_R11 = m_State.m_R[11];
            m_State.m_USR.m_R12 = m_State.m_R[12];
            m_State.m_USR.m_R13 = m_State.m_R[13];
            m_State.m_USR.m_R14 = m_State.m_R[14];
            break;
        case MODE_FIQ:
            m_State.m_FIQ.m_R8 = m_State.m_R[8];
            m_State.m_FIQ.m_R9 = m_State.m_R[9];
            m_State.m_FIQ.m_R10 = m_State.m_R[10];
            m_State.m_FIQ.m_R11 = m_State.m_R[11];
            m_State.m_FIQ.m_R12 = m_State.m_R[12];
            m_State.m_FIQ.m_R13 = m_State.m_R[13];
            m_State.m_FIQ.m_R14 = m_State.m_R[14];
            break;
        case MODE_IRQ:
            m_State.m_USR.m_R8 = m_State.m_R[8];
            m_State.m_USR.m_R9 = m_State.m_R[9];
            m_State.m_USR.m_R10 = m_State.m_R[10];
            m_State.m_USR.m_R11 = m_State.m_R[11];
            m_State.m_USR.m_R12 = m_State.m_R[12];
            m_State.m_IRQ.m_R13 = m_State.m_R[13];
            m_State.m_IRQ.m_R14 = m_State.m_R[14];
            break;
        case MODE_SVC:
            m_State.m_USR.m_R8 = m_State.m_R[8];
            m_State.m_USR.m_R9 = m_State.m_R[9];
            m_State.m_USR.m_R10 = m_State.m_R[10];
            m_State.m_USR.m_R11 = m_State.m_R[11];
            m_State.m_USR.m_R12 = m_State.m_R[12];
            m_State.m_SVC.m_R13 = m_State.m_R[13];
            m_State.m_SVC.m_R14 = m_State.m_R[14];
            break;
        case MODE_ABT:
            m_State.m_USR.m_R8 = m_State.m_R[8];
            m_State.m_USR.m_R9 = m_State.m_R[9];
            m_State.m_USR.m_R10 = m_State.m_R[10];
            m_State.m_USR.m_R11 = m_State.m_R[11];
            m_State.m_USR.m_R12 = m_State.m_R[12];
            m_State.m_ABT.m_R13 = m_State.m_R[13];
            m_State.m_ABT.m_R14 = m_State.m_R[14];
            break;
        case MODE_UND:
            m_State.m_USR.m_R8 = m_State.m_R[8];
            m_State.m_USR.m_R9 = m_State.m_R[9];
            m_State.m_USR.m_R10 = m_State.m_R[10];
            m_State.m_USR.m_R11 = m_State.m_R[11];
            m_State.m_USR.m_R12 = m_State.m_R[12];
            m_State.m_UND.m_R13 = m_State.m_R[13];
            m_State.m_UND.m_R14 = m_State.m_R[14];
            break;
        }
    }

    void ARM7::LoadRegisters()
    {
        switch (m_State.m_CPSR & MASK_MODE)
        {
        case MODE_USR:
        case MODE_SYS:
            m_State.m_R[8] = m_State.m_USR.m_R8;
            m_State.m_R[9] = m_State.m_USR.m_R9;
            m_State.m_R[10] = m_State.m_USR.m_R10;
            m_State.m_R[11] = m_State.m_USR.m_R11;
            m_State.m_R[12] = m_State.m_USR.m_R12;
            m_State.m_R[13] = m_State.m_USR.m_R13;
            m_State.m_R[14] = m_State.m_USR.m_R14;
            m_State.m_PSPSR = &m_State.m_SPSR[SPSR_DEF];
            break;
        case MODE_FIQ:
            m_State.m_R[8] = m_State.m_FIQ.m_R8;
            m_State.m_R[9] = m_State.m_FIQ.m_R9;
            m_State.m_R[10] = m_State.m_FIQ.m_R10;
            m_State.m_R[11] = m_State.m_FIQ.m_R11;
            m_State.m_R[12] = m_State.m_FIQ.m_R12;
            m_State.m_R[13] = m_State.m_FIQ.m_R13;
            m_State.m_R[14] = m_State.m_FIQ.m_R14;
            m_State.m_PSPSR = &m_State.m_SPSR[SPSR_FIQ];
            break;
        case MODE_IRQ:
            m_State.m_R[8] = m_State.m_USR.m_R8;
            m_State.m_R[9] = m_State.m_USR.m_R9;
            m_State.m_R[10] = m_State.m_USR.m_R10;
            m_State.m_R[11] = m_State.m_USR.m_R11;
            m_State.m_R[12] = m_State.m_USR.m_R12;
            m_State.m_R[13] = m_State.m_IRQ.m_R13;
            m_State.m_R[14] = m_State.m_IRQ.m_R14;
            m_State.m_PSPSR = &m_State.m_SPSR[SPSR_IRQ];
            break;
        case MODE_SVC:
            m_State.m_R[8] = m_State.m_USR.m_R8;
            m_State.m_R[9] = m_State.m_USR.m_R9;
            m_State.m_R[10] = m_State.m_USR.m_R10;
            m_State.m_R[11] = m_State.m_USR.m_R11;
            m_State.m_R[12] = m_State.m_USR.m_R12;
            m_State.m_R[13] = m_State.m_SVC.m_R13;
            m_State.m_R[14] = m_State.m_SVC.m_R14;
            m_State.m_PSPSR = &m_State.m_SPSR[SPSR_SVC];
            break;
        case MODE_ABT:
            m_State.m_R[8] = m_State.m_USR.m_R8;
            m_State.m_R[9] = m_State.m_USR.m_R9;
            m_State.m_R[10] = m_State.m_USR.m_R10;
            m_State.m_R[11] = m_State.m_USR.m_R11;
            m_State.m_R[12] = m_State.m_USR.m_R12;
            m_State.m_R[13] = m_State.m_ABT.m_R13;
            m_State.m_R[14] = m_State.m_ABT.m_R14;
            m_State.m_PSPSR = &m_State.m_SPSR[SPSR_ABT];
            break;
        case MODE_UND:
            m_State.m_R[8] = m_State.m_USR.m_R8;
            m_State.m_R[9] = m_State.m_USR.m_R9;
            m_State.m_R[10] = m_State.m_USR.m_R10;
            m_State.m_R[11] = m_State.m_USR.m_R11;
            m_State.m_R[12] = m_State.m_USR.m_R12;
            m_State.m_R[13] = m_State.m_UND.m_R13;
            m_State.m_R[14] = m_State.m_UND.m_R14;
            m_State.m_PSPSR = &m_State.m_SPSR[SPSR_UND];
            break;
        }
    }

    void ARM7::RaiseIRQ()
    {
        if ((m_State.m_CPSR & MASK_IRQD) == 0)
        {
            bool thumb = m_State.m_CPSR & MASK_THUMB;

            // "Useless" pipeline prefetch
            cycles += Memory::SequentialAccess(m_State.m_R[15], thumb ? ACCESS_HWORD : ACCESS_WORD);

            // Store return address in r14<irq>
            m_State.m_IRQ.m_R14 = m_State.m_R[15] - (thumb ? 4 : 8) + 4;

            // Save program status and switch mode
            m_State.m_SPSR[SPSR_IRQ] = m_State.m_CPSR;
            SaveRegisters();
            m_State.m_CPSR = (m_State.m_CPSR & ~(MASK_MODE | MASK_THUMB)) | MODE_IRQ | MASK_IRQD;
            LoadRegisters();

            // Jump to exception vector
            m_State.m_R[15] = EXCPT_INTERRUPT;
            m_Pipe.m_Index = 0;
            m_Pipe.m_Flush = false;
            RefillPipeline();

            // Emulate pipeline refill timings
            cycles += Memory::NonSequentialAccess(m_State.m_R[15], ACCESS_WORD) +
                      Memory::SequentialAccess(m_State.m_R[15] + 4, ACCESS_WORD);
        }
    }

    void ARM7::SWI(int number)
    {
        switch (number)
        {
        case 0x06:
        {
            if (m_State.m_R[1] != 0)
            {
                u32 mod = m_State.m_R[0] % m_State.m_R[1];
                u32 div = m_State.m_R[0] / m_State.m_R[1];
                m_State.m_R[0] = div;
                m_State.m_R[1] = mod;
            } else 
            {
                LOG(LOG_ERROR, "SWI6h: Attempted division by zero.");
            }
            break;
        }
        case 0x05:
            m_State.m_R[0] = 1;
            m_State.m_R[1] = 1;
        case 0x04:
            Memory::m_Interrupt.ime = 1;

            // If r0 is one IF must be cleared
            if (m_State.m_R[0] == 1)
                Memory::m_Interrupt.if_ = 0;

            // Sets GBA into halt state, waiting for specific interrupt(s) to occur.
            Memory::m_IntrWait = true;
            Memory::m_IntrWaitMask = m_State.m_R[1];
            Memory::m_HaltState = HALTSTATE_HALT;
            break;
        case 0x0B:
        {
            u32 source = m_State.m_R[0];
            u32 dest = m_State.m_R[1];
            u32 length = m_State.m_R[2] & 0xFFFFF;
            bool fixed = m_State.m_R[2] & (1 << 24) ? true : false;

            if (m_State.m_R[2] & (1 << 26))
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
            u32 source = m_State.m_R[0];
            u32 dest = m_State.m_R[1];
            u32 length = m_State.m_R[2] & 0xFFFFF;
            bool fixed = m_State.m_R[2] & (1 << 24) ? true : false;

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
            int amount = Memory::ReadWord(m_State.m_R[0]) >> 8;
            u32 source = m_State.m_R[0] + 4;
            u32 dest = m_State.m_R[1];

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

