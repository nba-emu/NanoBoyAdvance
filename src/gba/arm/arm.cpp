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

#include <cstring>
#include "arm.hpp"

namespace GBA
{
    arm::arm()
    {
        reset();
    }

    void arm::reset()
    {
        m_pipeline.m_index = 0;

        std::memset(m_reg, 0, sizeof(m_reg));
        std::memset(m_bank, 0, sizeof(m_bank));
        m_cpsr = MODE_SYS;
        m_spsr_ptr = &m_spsr[SPSR_DEF];

        // Skip bios boot logo
        m_reg[15] = 0x8000000;
        m_reg[13] = 0x3007F00;
        m_bank[BANK_SVC][BANK_R13] = 0x3007FE0;
        m_bank[BANK_IRQ][BANK_R13] = 0x3007FA0;
        RefillPipeline();
    }

    // Based on mGBA (endrift's) approach to banking.
    // https://github.com/mgba-emu/mgba/blob/master/src/arm/arm.c
    void arm::switch_mode(cpu_mode new_mode)
    {
        cpu_mode old_mode = static_cast<cpu_mode>(m_cpsr & MASK_MODE);

        if (new_mode == old_mode) return;

        cpu_bank new_bank = mode_to_bank(new_mode);
        cpu_bank old_bank = mode_to_bank(old_mode);

        if (new_bank != old_bank)
        {
            if (new_bank == BANK_FIQ || old_bank == BANK_FIQ)
            {
                int old_fiq_bank = old_bank == BANK_FIQ;
                int new_fiq_bank = new_bank == BANK_FIQ;

                // save general purpose registers to current bank.
                m_bank[old_fiq_bank][2] = m_reg[8];
                m_bank[old_fiq_bank][3] = m_reg[9];
                m_bank[old_fiq_bank][4] = m_reg[10];
                m_bank[old_fiq_bank][5] = m_reg[11];
                m_bank[old_fiq_bank][6] = m_reg[12];

                // restore general purpose registers from new bank.
                m_reg[8] = m_bank[new_fiq_bank][2];
                m_reg[9] = m_bank[new_fiq_bank][3];
                m_reg[10] = m_bank[new_fiq_bank][4];
                m_reg[11] = m_bank[new_fiq_bank][5];
                m_reg[12] = m_bank[new_fiq_bank][6];
            }

            // save SP and LR to current bank.
            m_bank[old_bank][BANK_R13] = m_reg[13];
            m_bank[old_bank][BANK_R14] = m_reg[14];

            // restore SP and LR from new bank.
            m_reg[13] = m_bank[new_bank][BANK_R13];
            m_reg[14] = m_bank[new_bank][BANK_R14];

            m_spsr_ptr = &m_spsr[new_bank];
        }

        // todo: use bit-utils for this
        m_cpsr = (m_cpsr & ~MASK_MODE) | (u32)new_mode;
    }

    void arm::step()
    {
        bool thumb = m_cpsr & MASK_THUMB;

        // Forcibly align r15
        m_reg[15] &= thumb ? ~1 : ~3;

        // Dispatch instruction loading and execution
        if (thumb)
        {
            if (m_pipeline.m_index == 0)
                m_pipeline.m_opcode[2] = Memory::ReadHWord(m_reg[15]);
            else
                m_pipeline.m_opcode[m_pipeline.m_index - 1] = Memory::ReadHWord(m_reg[15]);

            // Execute the thumb instruction via a method lut.
            u16 instruction = m_pipeline.m_opcode[m_pipeline.m_index];
            (this->*THUMB_TABLE[instruction >> 6])(instruction);
        }
        else
        {
            if (m_pipeline.m_index == 0)
                m_pipeline.m_opcode[2] = Memory::ReadWord(m_reg[15]);
            else
                m_pipeline.m_opcode[m_pipeline.m_index - 1] = Memory::ReadWord(m_reg[15]);

            Execute(m_pipeline.m_opcode[m_pipeline.m_index], Decode(m_pipeline.m_opcode[m_pipeline.m_index]));
        }

        if (m_pipeline.m_needs_flush)
        {
            m_pipeline.m_index = 0;
            m_pipeline.m_needs_flush = false;
            RefillPipeline();
            return;
        }

        // Update pipeline status
        m_pipeline.m_index = (m_pipeline.m_index + 1) % 3;

        // Update instruction pointer
        m_reg[15] += thumb ? 2 : 4;
    }

    void arm::raise_irq()
    {
        if ((m_cpsr & MASK_IRQD) == 0)
        {
            bool thumb = m_cpsr & MASK_THUMB;

            // "Useless" pipeline prefetch
            cycles += Memory::SequentialAccess(m_reg[15], thumb ? ACCESS_HWORD : ACCESS_WORD);

            // Store return address in r14<irq>
            m_bank[BANK_IRQ][BANK_R14] = m_reg[15] - (thumb ? 4 : 8) + 4;

            // Save program status and switch mode
            m_spsr[SPSR_IRQ] = m_cpsr;
            switch_mode(MODE_IRQ);
            m_cpsr = (m_cpsr & ~MASK_THUMB) | MASK_IRQD;

            // Jump to exception vector
            m_reg[15] = EXCPT_INTERRUPT;
            m_pipeline.m_index = 0;
            m_pipeline.m_needs_flush = false;
            RefillPipeline();

            // Emulate pipeline refill timings
            cycles += Memory::NonSequentialAccess(m_reg[15], ACCESS_WORD) +
                      Memory::SequentialAccess(m_reg[15] + 4, ACCESS_WORD);
        }
    }

    void arm::software_interrupt(int number)
    {
        switch (number)
        {
        case 0x06:
        {
            if (m_reg[1] != 0)
            {
                u32 mod = m_reg[0] % m_reg[1];
                u32 div = m_reg[0] / m_reg[1];
                m_reg[0] = div;
                m_reg[1] = mod;
            } else
            {
                LOG(LOG_ERROR, "SWI6h: Attempted division by zero.");
            }
            break;
        }
        case 0x05:
            m_reg[0] = 1;
            m_reg[1] = 1;
        case 0x04:
            Interrupt::WriteMasterEnableLow(1);
            Interrupt::WriteMasterEnableHigh(0);

            // If r0 is one IF must be cleared
            if (m_reg[0] == 1)
            {
                Interrupt::WriteInterruptFlagLow(0);
                Interrupt::WriteInterruptFlagHigh(0);
            }

            // Sets GBA into halt state, waiting for specific interrupt(s) to occur.
            Memory::m_IntrWait = true;
            Memory::m_IntrWaitMask = m_reg[1];
            Memory::m_HaltState = HALTSTATE_HALT;
            break;
        case 0x0B:
        {
            u32 source = m_reg[0];
            u32 dest = m_reg[1];
            u32 length = m_reg[2] & 0xFFFFF;
            bool fixed = m_reg[2] & (1 << 24) ? true : false;

            if (m_reg[2] & (1 << 26))
            {
                for (u32 i = 0; i < length; i++)
                {
                    write_word(dest, read_word(source));
                    dest += 4;

                    if (!fixed) source += 4;
                }
            }
            else
            {
                for (u32 i = 0; i < length; i++)
                {
                    write_hword(dest, read_hword(source));
                    dest += 2;

                    if (!fixed) source += 2;
                }
            }
            break;
        }
        case 0x0C:
        {
            u32 source = m_reg[0];
            u32 dest = m_reg[1];
            u32 length = m_reg[2] & 0xFFFFF;
            bool fixed = m_reg[2] & (1 << 24) ? true : false;

            for (u32 i = 0; i < length; i++)
            {
                write_word(dest, read_word(source));
                dest += 4;
                if (!fixed) source += 4;
            }
            break;
        }
        case 0x11:
        case 0x12:
        {
            int amount = Memory::ReadWord(m_reg[0]) >> 8;
            u32 source = m_reg[0] + 4;
            u32 dest = m_reg[1];

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
