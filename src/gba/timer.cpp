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
    void GBAMemory::RunTimer(int id, bool& overflow)
    {
        m_Timer[id].ticks = 0;

        if (m_Timer[id].count != 0xFFFF)
        {
            m_Timer[id].count++;
            return;
        }

        // Fires Interrupt if specified
        if (m_Timer[id].interrupt)
            m_Interrupt.if_ |= 8 << id;

        // Reset timer to its initial state
        m_Timer[id].count = m_Timer[id].reload;
        m_Timer[id].overflow = overflow = true;

        // Checks wether the current timer is a DMA FIFO clock.
        // If so transfer sample(s) of the controlled FIFO(s) to the audio chip.
        if (id == 0)
        {
            m_Audio.FifoLoadSample(0);
            m_Audio.FifoLoadSample(1);

            // TODO: Schedule DMA transfer here if FIFO needs refill.
        }
    }
}
