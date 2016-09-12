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


namespace NanoboyAdvance
{
    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     RunTimer
    ///
    ///////////////////////////////////////////////////////////
    void Memory::RunTimer()
    {
        bool overflow = false;

        for (int i = 0; i < 4; i++)
        {
            if (!m_Timer[i].enable ||
                    (m_Timer[i].countup && !overflow) ||
                    (!m_Timer[i].countup && ++m_Timer[i].ticks < TMR_CYCLES[m_Timer[i].clock]))
                continue;

            m_Timer[i].ticks = 0;

            if (m_Timer[i].count == 0xFFFF)
            {
                if (m_Timer[i].interrupt) m_Interrupt.if_ |= 8 << i;

                m_Timer[i].count    = m_Timer[i].reload;
                m_Timer[i].overflow = overflow = true;

                if (i == m_Audio.m_SoundControl.dma_timer[0]) m_Audio.FifoLoadSample(0);
                if (i == m_Audio.m_SoundControl.dma_timer[1]) m_Audio.FifoLoadSample(1);
            }
            else
            {
                m_Timer[i].count++;
                m_Timer[i].overflow = overflow = false;
            }
        }
    }
}
