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


#ifndef __NBA_QUADCHANNEL_H__
#define __NBA_QUADCHANNEL_H__


namespace NanoboyAdvance
{
    class QuadChannel
    {
    private:
        static constexpr float WAVE_DUTY_TABLE[4] = { 0.125, 0.25, 0.5, 0.75 };
        static constexpr int SWEEP_CLOCK_TABLE[8] = { 0, 32768, 98304, 131072, 163840, 196608, 229376 };

    public:
        enum SweepDirection
        {
            SWEEP_ADD = 0,
            SWEEP_SUB = 1
        };

        enum EnvelopeDirection
        {
            ENVELOPE_DECREASE = 0,
            ENVELOPE_INCREASE = 1
        };

        inline int GetFrequency()
        {
            return m_InitialFrequency;
        }

        inline void SetFrequency(int frequency)
        {
            m_InitialFrequency = m_CurrentFrequency = frequency;
        }

        inline int GetEnvelopeSweep()
        {
            return m_EnvelopeSweep;
        }

        inline void SetEnvelopeSweep(int sweep)
        {
            m_EnvelopeSweep = sweep;
            m_EnvelopeCycles = 0;
        }

        inline int GetVolume()
        {
            return m_LastWrittenVolume;
        }

        inline void SetVolume(int volume)
        {
            m_LastWrittenVolume = m_CurrentVolume = volume;
        }

        void Step();
        void Restart();
        float Next(int sample_rate);
        static float Generate(float t, float duty); // make private

        ///////////////////////////////////////////////////////////
        // Class members (Misc.)
        //
        ///////////////////////////////////////////////////////////
        bool m_Enable { true };
        int m_WavePatternDuty { 0 };
        int m_Sample { 0 }; // make private

        ///////////////////////////////////////////////////////////
        // Class members (Sweep)
        //
        ///////////////////////////////////////////////////////////
        int m_SweepTime { 0 };
        int m_SweepShift { 0 };
        SweepDirection m_SweepDirection { SWEEP_ADD };
    private:
        int m_SweepCycles { 0 };

        ///////////////////////////////////////////////////////////
        // Class members (Frequency)
        //
        ///////////////////////////////////////////////////////////
        int m_InitialFrequency { 0 };
        int m_LastFrequency { 0 };
        int m_CurrentFrequency { 0 };

    public:
        ///////////////////////////////////////////////////////////
        // Class members (Envelope)
        //
        ///////////////////////////////////////////////////////////
        EnvelopeDirection m_EnvelopeDirection { ENVELOPE_DECREASE };
    private:
        int m_EnvelopeSweep { 0 };
        int m_EnvelopeCycles { 0 };
        int m_LastWrittenVolume { 0 };
        int m_CurrentVolume { 0 };

        ///////////////////////////////////////////////////////////
        // Class members (Sound Length)
        //
        ///////////////////////////////////////////////////////////
        int m_LengthCycles { 0 };
    public:
        int m_Length { 0 };
        bool m_StopOnExpire { false };
    };
}


#endif // __NBA_QUADCHANNEL_H__
