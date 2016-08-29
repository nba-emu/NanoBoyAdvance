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


#include "audio.h"
#include "util/log.h"
#include <math.h>


namespace NanoboyAdvance
{
    Audio::Audio()
    {
        os[0] = new std::ofstream("fifo_a.raw", std::ofstream::trunc | std::ofstream::out | std::ofstream::binary);
        os[1] = new std::ofstream("fifo_b.raw", std::ofstream::trunc | std::ofstream::out | std::ofstream::binary);
    }

    Audio::~Audio()
    {
        os[0]->close();
        os[1]->close();
    }

    void Audio::Step()
    {
        u8 out = 0;

        m_Buffer.push_back(0);

        if (m_Buffer.size() == m_BufferSize)
        {
            double ratio[2];
            double source_index[2] {0, 0};

            ratio[0] = (double)m_FifoBuffer[0].size() / (double)m_BufferSize;
            ratio[1] = (double)m_FifoBuffer[1].size() / (double)m_BufferSize;

            for (int i = 0; i < m_BufferSize; i++)
            {
                s8 output = m_Buffer[i];

                for (int j = 0; j < 2; j++)
                {
                    // Get integral and fractional part of input index
                    double integral;
                    double fractional = modf(source_index[j], &integral);

                    if (integral >= m_FifoBuffer[j].size())
                        continue;

                    // Get current input sample
                    int8_t current = m_FifoBuffer[j][(int)integral];
                    int8_t between = current;

                    // Only if there is another sample
                    if (integral < m_FifoBuffer[j].size() - 1)
                    {
                        // Get the next sample
                        int8_t next = m_FifoBuffer[j][(int)integral + 1];

                        // Perform Cosinus Interpolation
                        fractional = (1 - cos(fractional * 3.141596)) / 2;
                        between = (current * (1 - fractional)) + (next * fractional);
                    }

                    output += between;
                    source_index[j] += ratio[j];
                }

                *os[0] << output;
            }

            m_FifoBuffer[0].clear();
            m_FifoBuffer[1].clear();
            m_Buffer.clear();
        }

        m_WaitCycles = 16780000 / m_SampleRate;
    }

    void Audio::FifoLoadSample(int fifo)
    {
        m_FifoBuffer[fifo].push_back(m_FIFO[fifo].Dequeue());
    }
}
