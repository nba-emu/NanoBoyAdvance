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


#ifndef __NBA_WAVECHANNEL_H__
#define __NBA_WAVECHANNEL_H__


#include "util/types.h"


namespace NanoboyAdvance
{
    class WaveChannel
    {
    private:
        static constexpr float WAVE_VOLUME[] = { 0, 1, 0.5, 0.25 };

    public:
        void Step();
        void Restart();
        float Next(int sample_rate);

        ///////////////////////////////////////////////////////////
        // Class members (Misc.)
        //
        ///////////////////////////////////////////////////////////
        bool m_Enable   { true };
        bool m_Playback { false };
        int m_Frequency { 0 };
        int m_Sample    { 0 }; // make private
        int m_Volume    { 0 };
        u8 m_WaveRAM[0x40];

        ///////////////////////////////////////////////////////////
        // Class members (Sound Length)
        //
        ///////////////////////////////////////////////////////////
        int m_LengthCycles  { 0 };
        int m_Length        { 0 };
        bool m_StopOnExpire { false };
    };
}


#endif // __NBA_WAVECHANNEL_H__
