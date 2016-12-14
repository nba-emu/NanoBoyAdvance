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

#include <cstring>

namespace GBA
{
    enum cpu_mode
    {
        MODE_USR = 0x10,
        MODE_FIQ = 0x11,
        MODE_IRQ = 0x12,
        MODE_SVC = 0x13,
        MODE_ABT = 0x17,
        MODE_UND = 0x1B,
        MODE_SYS = 0x1F
    };

    enum cpu_bank
    {
        BANK_NONE,
        BANK_FIQ,
        BANK_SVC,
        BANK_ABT,
        BANK_IRQ,
        BANK_UND,
        BANK_COUNT
    };

    enum cpu_bank_register
    {
        BANK_R13 = 0,
        BANK_R14 = 1
    };

    enum status_mask
    {
        MASK_MODE  = 0x1F,
        MASK_THUMB = 0x20,
        MASK_FIQD  = 0x40,
        MASK_IRQD  = 0x80,
        MASK_VFLAG = 0x10000000,
        MASK_CFLAG = 0x20000000,
        MASK_ZFLAG = 0x40000000,
        MASK_NFLAG = 0x80000000
    };

    enum exception_vector
    {
        EXCPT_RESET     = 0x00,
        EXCPT_UNDEFINED = 0x04,
        EXCPT_SWI       = 0x08,
        EXCPT_PREFETCH_ABORT = 0x0C,
        EXCPT_DATA_ABORT     = 0x10,
        EXCPT_INTERRUPT      = 0x18,
        EXCPT_FAST_INTERRUPT = 0x1C
    };

    enum saved_status_register
    {
        SPSR_DEF = 0,
        SPSR_FIQ = 1,
        SPSR_SVC = 2,
        SPSR_ABT = 3,
        SPSR_IRQ = 4,
        SPSR_UND = 5,
        SPSR_COUNT = 6
    };

    struct state
    {
        u32 m_reg[16];

        u32 m_bank[BANK_COUNT][7];

        u32 m_cpsr;
        u32 m_spsr[SPSR_COUNT];
        u32* m_spsr_ptr; // pointer to current SPSR.

        struct
        {
            u32 m_opcode[3];
            int m_index;
            bool m_needs_flush;
        } m_pipeline;

        state() { reset(); }

        void reset()
        {
            std::memset(this, 0, sizeof(state));

            m_cpsr = MODE_SYS;
            m_spsr_ptr = &m_spsr[SPSR_DEF];
        }

        // Inspired by mGBA (endrift's) approach to banking.
        // https://github.com/mgba-emu/mgba/blob/master/src/arm/arm.c
        cpu_bank mode_to_bank(cpu_mode mode)
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
                // TODO: log error
                return BANK_NONE;
            }
        }

        void switch_mode(cpu_mode new_mode)
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
    };
}
