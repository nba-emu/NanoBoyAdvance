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


#include "memory.h"


namespace GBA
{
    constexpr int Timer::TMR_CYCLES[4];

    void Timer::Increment(bool& overflow)
    {
        m_ticks = 0;

        if (m_count == 0xFFFF)
        {
            if (m_interrupt) Memory::m_Interrupt.if_ |= 8 << m_id;

            m_count    = m_reload;
            m_overflow = overflow = true;

            if (m_id == Memory::m_Audio.m_SoundControl.dma_timer[0]) Memory::m_Audio.FifoLoadSample(0);
            if (m_id == Memory::m_Audio.m_SoundControl.dma_timer[1]) Memory::m_Audio.FifoLoadSample(1);
        }
        else
        {
            m_count++;
            m_overflow = overflow = false;
        }
    }
}
