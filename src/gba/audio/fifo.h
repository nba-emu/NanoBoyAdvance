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


#ifndef __NBA_AUDIO_FIFO_H__
#define __NBA_AUDIO_FIFO_H__


#include "common/types.h"


namespace GBA
{
    ///////////////////////////////////////////////////////////
    /// \file    audio_fifo.h
    /// \author  Frederic Meyer
    /// \date    August 27th, 2016
    /// \class   FIFO
    /// \brief   Serves as DMA Audio FIFO.
    ///
    ///////////////////////////////////////////////////////////
    class FIFO
    {
    public:
        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 27th, 2016
        /// \fn      RequiresData
        /// \brief   Determines if the FIFO must be refilled.
        ///
        /// \returns  Wether the FIFO must be refilled.
        ///
        ///////////////////////////////////////////////////////////
        inline bool RequiresData()
        {
            return m_BufferIndex <= 16;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 27th, 2016
        /// \fn      Enqueue
        /// \brief   Enqueues one sample to the FIFO.
        ///
        /// \param  data  The sample.
        ///
        ///////////////////////////////////////////////////////////
        inline void Enqueue(u8 data)
        {
            if (m_BufferIndex < 32)
                m_Buffer[m_BufferIndex++] = (s8)data;
        }

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    August 27th, 2016
        /// \fn      Dequeue
        /// \brief   Dequeues one sample from the FIFO.
        ///
        /// \returns  The sample.
        ///
        ///////////////////////////////////////////////////////////
        inline s8 Dequeue()
        {
            s8 value;

            if (m_BufferIndex == 0)
                return 0;

            value = m_Buffer[0];

            for (int i = 1; i < m_BufferIndex; i++)
                m_Buffer[i - 1] = m_Buffer[i];

            m_BufferIndex--;

            return value;
        }

        ///////////////////////////////////////////////////////////
        /// \fn      Reset
        /// \brief   Resets the FIFO state.
        ///
        ///////////////////////////////////////////////////////////
        inline void Reset()
        {
            m_BufferIndex = 0;
        }

    private:
        ///////////////////////////////////////////////////////////
        /// Class members
        ///
        ///////////////////////////////////////////////////////////
        s8 m_Buffer[32];
        int m_BufferIndex { 0 };
    };
}

#endif // __NBA_AUDIO_FIFO_H__
