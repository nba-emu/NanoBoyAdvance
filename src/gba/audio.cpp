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
#include "config/config.h"
#include <math.h>


namespace NanoboyAdvance
{
    Audio::Audio()
    {
        Config config("config.sml");
        m_SampleRate = config.ReadInt("Audio::Quality", "SampleRate");
    }

    void Audio::Step()
    {
        m_Buffer.push_back(0);
        m_WaitCycles = 16780000 / m_SampleRate;
    }

    void Audio::FifoLoadSample(int fifo)
    {
        m_FifoBuffer[fifo].push_back(m_FIFO[fifo].Dequeue());
    }

    void AudioCallback(Audio* audio, s8* stream, int length)
    {
        double ratio[2];
        double source_index[2] {0, 0};
        int actual_length = audio->m_Buffer.size();

        length = length / 2; // we have two channels, but we want the actual sample count.
        ratio[0] = (double)audio->m_FifoBuffer[0].size() / (double)actual_length;
        ratio[1] = (double)audio->m_FifoBuffer[1].size() / (double)actual_length;

        for (int i = 0; i < length; i++)
        {
            s8 output[2];

            if (i >= actual_length)
            {
                // TODO: Fill rest of the buffer and exit directly.
                stream[i * 2] = 0;
                stream[i * 2 + 1] = 0;
                continue;
            }

            //output[0] = audio->m_Buffer[i];
            output[0] = output[1] = 0;

            for (int j = 0; j < 2; j++)
            {
                if (!audio->m_SoundControl.dma_enable_left[j] &&
                    !audio->m_SoundControl.dma_enable_right[j])
                    continue;

                // Get integral and fractional part of input index
                double integral;
                double fractional = modf(source_index[j], &integral);

                if (integral >= audio->m_FifoBuffer[j].size())
                    continue;

                // Get current input sample
                int8_t current = audio->m_FifoBuffer[j][(int)integral];
                int8_t between = current;

                // Only if there is another sample
                if (integral < audio->m_FifoBuffer[j].size() - 1)
                {
                    // Get the next sample
                    int8_t next = audio->m_FifoBuffer[j][(int)integral + 1];

                    // Perform Cosinus Interpolation
                    fractional = (1 - cos(fractional * 3.141596)) / 2;
                    between = (current * (1 - fractional)) + (next * fractional);
                }

                if (audio->m_SoundControl.dma_enable_left[j])
                    output[0] += between / (2 - audio->m_SoundControl.dma_volume[j]);

                if (audio->m_SoundControl.dma_enable_right[j])
                    output[1] += between / (2 - audio->m_SoundControl.dma_volume[j]);

                source_index[j] += ratio[j];
            }

            stream[i * 2] = output[0];
            stream[i * 2 + 1] = output[1];
        }

        if (actual_length > length)
        {
            if (audio->m_FifoBuffer[0].size() > source_index[0])
                audio->m_FifoBuffer[0].erase(audio->m_FifoBuffer[0].begin(), audio->m_FifoBuffer[0].begin()+source_index[0]);

            if (audio->m_FifoBuffer[1].size() > source_index[1])
                audio->m_FifoBuffer[1].erase(audio->m_FifoBuffer[1].begin(), audio->m_FifoBuffer[1].begin()+source_index[1]);

            audio->m_Buffer.erase(audio->m_Buffer.begin(), audio->m_Buffer.begin()+length);
        }
        else
        {
            audio->m_FifoBuffer[0].clear();
            audio->m_FifoBuffer[1].clear();
            audio->m_Buffer.clear();
        }
    }
}
