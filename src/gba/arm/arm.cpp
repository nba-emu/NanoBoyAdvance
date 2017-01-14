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

        // skip bios boot logo
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

        m_cpsr = (m_cpsr & ~MASK_MODE) | (u32)new_mode;
    }

    void arm::step()
    {
        bool thumb = m_cpsr & MASK_THUMB;
        cycles++; /* NOTE: preliminary */

        // Dispatch instruction loading and execution
        if (thumb)
        {
            m_reg[15] &= ~1;

            if (m_pipeline.m_index == 0)
                m_pipeline.m_opcode[2] = read_hword(m_reg[15]);
            else
                m_pipeline.m_opcode[m_pipeline.m_index - 1] = read_hword(m_reg[15]);

            thumb_execute(m_pipeline.m_opcode[m_pipeline.m_index]);
        }
        else
        {
            m_reg[15] &= ~3;

            if (m_pipeline.m_index == 0)
                m_pipeline.m_opcode[2] = read_word(m_reg[15]);
            else
                m_pipeline.m_opcode[m_pipeline.m_index - 1] = read_word(m_reg[15]);

            arm_execute(m_pipeline.m_opcode[m_pipeline.m_index]);
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
        if (m_cpsr & MASK_IRQD) return;

        bool thumb = m_cpsr & MASK_THUMB;

        // store return address in r14<irq>
        m_bank[BANK_IRQ][BANK_R14] = m_reg[15] - (thumb ? 4 : 8) + 4;

        // save program status and switch mode
        m_spsr[SPSR_IRQ] = m_cpsr;
        switch_mode(MODE_IRQ);
        m_cpsr = (m_cpsr & ~MASK_THUMB) | MASK_IRQD;

        // jump to exception vector
        m_reg[15] = EXCPT_INTERRUPT;
        m_pipeline.m_index = 0;
        m_pipeline.m_needs_flush = false;
        RefillPipeline();
    }
}
