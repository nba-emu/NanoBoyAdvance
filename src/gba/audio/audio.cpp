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


namespace GBA
{
    constexpr float Audio::PSG_VOLUME[];
    constexpr float Audio::DMA_VOLUME[];
    const int Audio::DMA_SCALE;

    Audio::Audio()
    {
        Config config("config.sml");
        m_SampleRate = config.ReadInt("Audio::Quality", "SampleRate");
    }

    void Audio::Step()
    {
        s16 out = 0;

        float volume_left = (float)m_SoundControl.master_volume_left / 7;
        float volume_right = (float)m_SoundControl.master_volume_right / 7;

        s16 psg1 = m_QuadChannel[0].m_Enable ? m_QuadChannel[0].Next(m_SampleRate) : 0;
        s16 psg2 = m_QuadChannel[1].m_Enable ? m_QuadChannel[1].Next(m_SampleRate) : 0;
        s16 psg3 = m_WaveChannel.m_Enable ? m_WaveChannel.Next(m_SampleRate) : 0;

        m_Mutex.lock();

        // Mix left channel
        if (m_SoundControl.enable_left[0]) out += psg1;
        if (m_SoundControl.enable_left[1]) out += psg2;
        if (m_SoundControl.enable_left[2]) out += psg3;
        m_PsgBuffer[0].push_back(out * volume_left);
        out = 0;

        // Mix right channel
        if (m_SoundControl.enable_right[0]) out += psg1;
        if (m_SoundControl.enable_right[1]) out += psg2;
        if (m_SoundControl.enable_right[2]) out += psg3;
        m_PsgBuffer[1].push_back(out * volume_right);

        m_Mutex.unlock();

        m_WaitCycles = 16780000 / m_SampleRate;
    }

    void Audio::FifoLoadSample(int fifo)
    {
        m_Mutex.lock();
        m_FifoBuffer[fifo].push_back(m_FIFO[fifo].Dequeue());
        m_Mutex.unlock();
    }

    void AudioCallback(Audio* audio, u16* stream, int length)
    {
        double ratio[2];
        int actual_length;
        double source_index[2] {0, 0};
        u16 last_sample[2];
        float psg_amplitude = audio->PSG_VOLUME[audio->m_SoundControl.volume];

        audio->m_Mutex.lock();

        length = length / 4; // 2 channels and length is in bytes, while we want in hwords.
        actual_length = audio->m_PsgBuffer[0].size();
        ratio[0] = (double)audio->m_FifoBuffer[0].size() / (double)actual_length;
        ratio[1] = (double)audio->m_FifoBuffer[1].size() / (double)actual_length;

        for (int i = 0; i < length; i++)
        {
            s16 output[2];

            // If more data is requested than we have, fill the ramaining samples
            // atleast with the last sample to prevent annoying clicking.
            if (i >= actual_length)
            {
                stream[i * 2]     = last_sample[0];
                stream[i * 2 + 1] = last_sample[1];
                continue;
            }

            output[0] = audio->m_PsgBuffer[0][i] * psg_amplitude;
            output[1] = audio->m_PsgBuffer[1][i] * psg_amplitude;

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
                int8_t sample = current;

                // Only if there is another sample
                if (integral < audio->m_FifoBuffer[j].size() - 1)
                {
                    // Get the next sample
                    int8_t next = audio->m_FifoBuffer[j][(int)integral + 1];

                    // Perform Cosinus Interpolation
                    fractional = (1 - cos(fractional * 3.141596)) / 2;
                    sample = (current * (1 - fractional)) + (next * fractional);
                }

                float sample_amplitude = audio->DMA_SCALE *
                        audio->DMA_VOLUME[audio->m_SoundControl.dma_volume[j]];

                if (audio->m_SoundControl.dma_enable_left[j])
                    output[0] += sample * sample_amplitude;

                if (audio->m_SoundControl.dma_enable_right[j])
                    output[1] += sample * sample_amplitude;

                source_index[j] += ratio[j];
            }

            // Makeup gain (BIAS) and clipping emulation.
            output[0] += (audio->m_SOUNDBIAS & 0x3FF);
            output[1] += (audio->m_SOUNDBIAS & 0x3FF);
            if (output[0] < 0)     output[0] = 0;
            if (output[1] < 0)     output[1] = 0;
            if (output[0] > 0x3FF) output[0] = 0x3FF;
            if (output[1] > 0x3FF) output[1] = 0x3FF;

            // Bitcrush audio
            int crush_amount = (audio->m_SOUNDBIAS >> 14) & 3;
            output[0] = (output[0] >> crush_amount) << crush_amount;
            output[1] = (output[1] >> crush_amount) << crush_amount;

            stream[i * 2]     = last_sample[0] = (u16)output[0] * 64;
            stream[i * 2 + 1] = last_sample[1] = (u16)output[1] * 64;
        }

        if (actual_length > length)
        {
            if (audio->m_FifoBuffer[0].size() > source_index[0])
                audio->m_FifoBuffer[0].erase(audio->m_FifoBuffer[0].begin(), audio->m_FifoBuffer[0].begin()+source_index[0]);

            if (audio->m_FifoBuffer[1].size() > source_index[1])
                audio->m_FifoBuffer[1].erase(audio->m_FifoBuffer[1].begin(), audio->m_FifoBuffer[1].begin()+source_index[1]);

            audio->m_PsgBuffer[0].erase(audio->m_PsgBuffer[0].begin(), audio->m_PsgBuffer[0].begin()+length);
            audio->m_PsgBuffer[1].erase(audio->m_PsgBuffer[1].begin(), audio->m_PsgBuffer[1].begin()+length);
        }
        else
        {
            audio->m_FifoBuffer[0].clear();
            audio->m_FifoBuffer[1].clear();
            audio->m_PsgBuffer[0].clear();
            audio->m_PsgBuffer[1].clear();
        }

        audio->m_Mutex.unlock();
    }

    float Audio::ConvertFrequency(int frequency)
    {
        return 131072 / (2048 - frequency);
    }
}
