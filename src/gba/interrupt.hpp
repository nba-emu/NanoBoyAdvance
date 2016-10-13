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


#ifndef __NBA_INTERRUPT_HPP__
#define __NBA_INTERRUPT_HPP__

#include "common/types.h"

namespace GBA
{
    class Interrupt
    {
    private:
        static u16 m_master_enable;
        static u16 m_interrupt_flag;
        static u16 m_interrupt_enable;

    public:
        static void Reset()
        {
            m_master_enable = 0;
            m_interrupt_flag = 0;
            m_interrupt_enable = 0;
        }

        static bool GetMasterEnable()
        {
            return m_master_enable;
        }

        static bool GetRequestedAndEnabled()
        {
            return m_interrupt_flag & m_interrupt_enable;
        }

        static void RequestInterrupt(int id)
        {
            m_interrupt_flag |= (1 << id);
        }

        static u16 ReadMasterEnableLow()
        {
            return m_master_enable & 0xFF;
        }

        static u16 ReadMasterEnableHigh()
        {
            return m_master_enable >> 8;
        }

        static void WriteMasterEnableLow(u16 value)
        {
            m_master_enable = (m_master_enable & 0xFF00) | value;
        }

        static void WriteMasterEnableHigh(u16 value)
        {
            m_master_enable = (m_master_enable & 0x00FF) | (value << 8);
        }

        static u16 ReadInterruptFlagLow()
        {
            return m_interrupt_flag & 0xFF;
        }

        static u16 ReadInterruptFlagHigh()
        {
            return m_interrupt_flag >> 8;
        }

        static void WriteInterruptFlagLow(u16 value)
        {
            m_interrupt_flag &= ~value;
        }

        static void WriteInterruptFlagHigh(u16 value)
        {
            m_interrupt_flag &= ~(value << 8);
        }

        static u16 ReadInterruptEnableLow()
        {
            return m_interrupt_enable & 0xFF;
        }

        static u16 ReadInterruptEnableHigh()
        {
            return m_interrupt_enable >> 8;
        }

        static void WriteInterruptEnableLow(u16 value)
        {
            m_interrupt_enable = (m_interrupt_enable & 0xFF00) | value;
        }

        static void WriteInterruptEnableHigh(u16 value)
        {
            m_interrupt_enable = (m_interrupt_enable & 0x00FF) | (value << 8);
        }
    };
}

#endif  // __NBA_INTERRUPT_HPP__
