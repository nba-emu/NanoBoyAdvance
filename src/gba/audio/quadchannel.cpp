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
#include "quadchannel.h"
#include <cmath>


namespace GBA
{
    class Audio { public: static float ConvertFrequency(int frequency); };

    constexpr float QuadChannel::WAVE_DUTY_TABLE[4];
    constexpr int QuadChannel::SWEEP_CLOCK_TABLE[8];

    void QuadChannel::Step()
    {
        int sweep_clock = SWEEP_CLOCK_TABLE[m_SweepTime];
        int envelope_clock = (int)(m_EnvelopeSweep * (1.0 / 64.0) * 4194304.0);

        // Apply sound frequency sweep/modulation.
        if (sweep_clock != 0)
        {
            m_SweepCycles++;

            if (m_SweepCycles >= sweep_clock)
            {
                m_SweepCycles = 0;
                m_LastFrequency = m_CurrentFrequency;

                if (m_SweepDirection == SWEEP_ADD)
                    m_CurrentFrequency = m_LastFrequency + m_LastFrequency / (2 << m_SweepShift);
                else
                    m_CurrentFrequency = m_LastFrequency - m_LastFrequency / (2 << m_SweepShift);
            }
        }

        // Apply sound amplitude sweep/modulation.
        if (m_EnvelopeSweep != 0)
        {
            m_EnvelopeCycles++;

            if (m_EnvelopeCycles >= envelope_clock)
            {
                m_EnvelopeCycles = 0;

                if (m_EnvelopeDirection == ENVELOPE_INCREASE)
                {
                    if (m_CurrentVolume != 15) m_CurrentVolume++;
                }
                else
                {
                    if (m_CurrentVolume != 0) m_CurrentVolume--;
                }
            }
        }

        if (m_StopOnExpire) m_LengthCycles++;
    }

    void QuadChannel::Restart()
    {
        m_CurrentFrequency = m_InitialFrequency;
        m_SweepCycles = 0;
        m_CurrentVolume = m_LastWrittenVolume;
        m_LengthCycles = 0;
        m_EnvelopeCycles = 0;
    }

    float QuadChannel::Next(int sample_rate)
    {
        int max_cycles = (64 - m_Length) * (1.0 / 256.0) * 4194304;

        if (m_StopOnExpire && m_LengthCycles > max_cycles)
            return 0;

        float amplitude = (float)m_CurrentVolume * (1.0 / 16.0);
        float frequency = Audio::ConvertFrequency(m_CurrentFrequency);
        float sample_position = (float)((2 * M_PI * m_Sample * frequency) / sample_rate);
        float value = (float)(Generate(sample_position, WAVE_DUTY_TABLE[m_WavePatternDuty]));

        if (++m_Sample >= sample_rate) m_Sample = 0;

        return amplitude * value;
    }

    float QuadChannel::Generate(float t, float duty)
    {
        t = std::fmod(t, 2 * M_PI);

        if (t <= 2 * M_PI * duty)
            return 127;
        return -128;
    }
}
