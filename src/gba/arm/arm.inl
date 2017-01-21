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

namespace armigo
{
    inline cpu_bank arm::mode_to_bank(cpu_mode mode)
    {
        switch (mode)
        {
        case MODE_USR:
        case MODE_SYS:
            return BANK_NONE;
        case MODE_FIQ:
            return BANK_FIQ;
        case MODE_IRQ:
            return BANK_IRQ;
        case MODE_SVC:
            return BANK_SVC;
        case MODE_ABT:
            return BANK_ABT;
        case MODE_UND:
            return BANK_UND;
        default:
            return BANK_NONE;
        }
    }

    inline void arm::refill_pipeline()
    {
        if (m_cpsr & MASK_THUMB)
        {
            m_pipeline.m_opcode[0] = read_hword(m_reg[15]);
            m_pipeline.m_opcode[1] = read_hword(m_reg[15] + 2);
            m_reg[15] += 4;
        }
        else
        {
            m_pipeline.m_opcode[0] = read_word(m_reg[15]);
            m_pipeline.m_opcode[1] = read_word(m_reg[15] + 4);
            m_reg[15] += 8;
        }
    }
}
