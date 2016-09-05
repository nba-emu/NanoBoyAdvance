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


#define _USE_MATH_DEFINES
#include "wavechannel.h"
#include <cmath>


namespace NanoboyAdvance
{
    class Audio { public: static float ConvertFrequency(int frequency); };

    constexpr float WaveChannel::WAVE_VOLUME[];

    void WaveChannel::Step()
    {
        if (m_StopOnExpire) m_LengthCycles++;
    }

    void WaveChannel::Restart()
    {
        m_LengthCycles = 0;
    }

    float WaveChannel::Next(int sample_rate)
    {
        if (!m_Playback) return 0;
        if (m_StopOnExpire && m_LengthCycles > m_Length) return 0;

        float frequency = Audio::ConvertFrequency(m_Frequency);
        float index = fmod((5.093108 * m_Sample * frequency * M_PI) / sample_rate, 32);

        // Find better scaling algorithm...
        return (float)(m_WaveRAM[(int)index] - 7) * 16;
    }
}
