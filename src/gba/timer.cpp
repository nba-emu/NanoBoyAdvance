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
#include <fstream>

namespace NanoboyAdvance
{
    ///////////////////////////////////////////////////////////
    /// \author Frederic Meyer
    /// \date   July 31th, 2016
    /// \fn     RunTimer
    ///
    ///////////////////////////////////////////////////////////
    void GBAMemory::RunTimer()
    {
        bool overflow = false;

        for (int i = 0; i < 4; i++)
        {
            if (!m_Timer[i].enable) continue;

            if ((m_Timer[i].countup && overflow) ||
                (!m_Timer[i].countup && ++m_Timer[i].ticks >= tmr_cycles[m_Timer[i].clock]))
            {
                m_Timer[i].ticks = 0;

                if (m_Timer[i].count != 0xFFFF)
                {
                    m_Timer[i].count++;
                    overflow = false;
                }
                else
                {
                    if (m_Timer[i].interrupt) m_Interrupt->if_ |= 8 << i;
                    m_Timer[i].count = m_Timer[i].reload;
                    m_Timer[i].overflow = true;
                    overflow = true;

                    if (i == 0)
                    {
                        std::ofstream os1("fifo_a.raw", std::ofstream::out | std::ofstream::app | std::ofstream::binary);
                        std::ofstream os2("fifo_b.raw", std::ofstream::out | std::ofstream::app | std::ofstream::binary);
                        os1 << m_FIFO[0].Dequeue();
                        os2 << m_FIFO[1].Dequeue();
                        os1.close();
                        os2.close();
//                        LOG(LOG_INFO, "FIFOA: %d", m_FIFO[0].Dequeue());
//                        LOG(LOG_INFO, "FIFOB: %d", m_FIFO[1].Dequeue());
                    }
                }
            }
        }
    }
}
