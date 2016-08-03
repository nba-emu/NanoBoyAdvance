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
    /// \fn     RunDMA
    ///
    ///////////////////////////////////////////////////////////
    void GBAMemory::RunDMA()
    {
        // TODO: FIFO A/B and Video Capture
        for (int i = 0; i < 4; i++)
        {
            bool start = false;

            if (!m_DMA[i].enable) continue;

            switch (m_DMA[i].start_time)
            {
            case StartTime::Immediate:
                start = true;
                break;
            case StartTime::VBlank:
                if (m_Video->m_VBlankDMA)
                {
                    start = true;
                    m_Video->m_VBlankDMA = false;
                }
                break;
            case StartTime::HBlank:
                if (m_Video->m_HBlankDMA)
                {
                    start = true;
                    m_Video->m_HBlankDMA = false;
                }
                break;
            case StartTime::Special:
                #ifdef DEBUG
                ASSERT(i == 3, LOG_ERROR, "DMA: Video Capture Mode not supported.");
                #endif
                break;
            }

            m_DidTransfer = start;
            m_DMACycles = 2;

            if (start)
            {
                AddressControl dest_control = m_DMA[i].dest_control;
                AddressControl source_control = m_DMA[i].source_control;
                bool transfer_words = m_DMA[i].size == TransferSize::Word;

                #if DEBUG
                u32 value = ReadHWord(m_DMA[i].source);
                LOG(LOG_INFO, "DMA%d: s=%x d=%x c=%x count=%x l=%d v=%x", i, m_DMA[i].source_int,
                               m_DMA[i].dest_int, 0, m_DMA[i].count_int, m_Video->m_VCount, value);
                ASSERT(m_DMA[i].gamepack_drq, LOG_ERROR, "Game Pak DRQ not supported.");
                #endif

                // Run as long as there is data to transfer
                while (m_DMA[i].count_int != 0)
                {
                    // Transfer either Word or HWord
                    if (transfer_words)
                    {
                        WriteWord(m_DMA[i].dest_int & ~3, ReadWord(m_DMA[i].source_int & ~3));
                        m_DMACycles += SequentialAccess(m_DMA[i].dest_int, AccessSize::Word) +
                                      SequentialAccess(m_DMA[i].source_int, AccessSize::Word);
                    }
                    else
                    {
                        WriteHWord(m_DMA[i].dest_int & ~1, ReadHWord(m_DMA[i].source_int & ~1));
                        m_DMACycles += SequentialAccess(m_DMA[i].dest_int, AccessSize::Hword) +
                                      SequentialAccess(m_DMA[i].source_int, AccessSize::Hword);
                    }

                    // Update destination address
                    if (dest_control == AddressControl::Increment || dest_control == AddressControl::Reload)
                        m_DMA[i].dest_int += transfer_words ? 4 : 2;
                    else if (dest_control == AddressControl::Decrement)
                        m_DMA[i].dest_int -= transfer_words ? 4 : 2;

                    // Update source address
                    if (source_control == AddressControl::Increment || source_control == AddressControl::Reload)
                        m_DMA[i].source_int += transfer_words ? 4 : 2;
                    else if (source_control == AddressControl::Decrement)
                        m_DMA[i].source_int -= transfer_words ? 4 : 2;

                    // Update count
                    m_DMA[i].count_int--;
                }

                // Reschedule the DMA as specified or disable it
                if (m_DMA[i].repeat)
                {
                    m_DMA[i].count_int = m_DMA[i].count & dma_count_mask[i];
                    if (m_DMA[i].count_int == 0)
                        m_DMA[i].count_int = dma_count_mask[i] + 1;
                    if (dest_control == AddressControl::Reload)
                        m_DMA[i].dest_int  = m_DMA[i].dest & dma_dest_mask[i];
                }
                else
                {
                    m_DMA[i].enable = false;
                }

                // Raise DMA interrupt if enabled
                if (m_DMA[i].interrupt)
                    m_Interrupt->if_ |= 256 << i;
            }
        }
    }
}
