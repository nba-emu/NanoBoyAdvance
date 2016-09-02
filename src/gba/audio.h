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

#ifndef __NBA_AUDIO_H__
#define __NBA_AUDIO_H__


#include "fifo.h"
#include "quadchannel.h"
#include <fstream>
#include <vector>


namespace NanoboyAdvance
{
    ///////////////////////////////////////////////////////////
    /// \file    audio.h
    /// \class   Audio
    /// \brief   Serves as Audio controller.
    ///
    ///////////////////////////////////////////////////////////
    class Audio
    {
    public:
        ///////////////////////////////////////////////////////////
        /// \struct  SoundControl
        /// \brief   Holds SOUNDCNT information
        ///
        ///////////////////////////////////////////////////////////
        struct SoundControl
        {
            int  volume                 { 0 };
            int  master_volume_left     { 0 };
            int  master_volume_right    { 0 };
            bool enable_left[4]         { false, false, false, false };
            bool enable_right[4]        { false, false, false, false };
            int  dma_timer[2]           { 0, 0 };
            int  dma_volume[2]          { 0, 0 };
            bool dma_enable_left[2]     { false, false };
            bool dma_enable_right[2]    { false, false };
        };

        ///////////////////////////////////////////////////////////
        /// \fn      Audio
        /// \brief   Constructor
        ///
        ///////////////////////////////////////////////////////////
        Audio();

        ///////////////////////////////////////////////////////////
        /// \fn      Step
        /// \brief   Perforn audio cyclic tasks.
        ///
        ///////////////////////////////////////////////////////////
        void Step();

        ///////////////////////////////////////////////////////////
        /// \fn      FifoLoadSample
        /// \brief   Load a sample from the given FIFO.
        /// \param   fifo  FIFO ID
        ///
        ///////////////////////////////////////////////////////////
        void FifoLoadSample(int fifo);

        ///////////////////////////////////////////////////////////
        /// \fn      ConvertFrequency
        /// \brief   Load a sample from the given FIFO.
        ///
        ///////////////////////////////////////////////////////////
        static float ConvertFrequency(int frequency);

    public:
        ///////////////////////////////////////////////////////////
        /// Class members
        ///
        ///////////////////////////////////////////////////////////
        FIFO m_FIFO[2];
        QuadChannel m_QuadChannel[2];
        SoundControl m_SoundControl;
        std::vector<s8> m_Buffer;
        std::vector<s8> m_FifoBuffer[2];
        int m_WaitCycles { 0 };
    private:
        int m_SampleRate { 0 };
    };

    ///////////////////////////////////////////////////////////
    /// \fn     AudioCallback
    /// \brief  Called by an Audio Adapter to request audio data.
    ///
    ///////////////////////////////////////////////////////////
    void AudioCallback(Audio* audio, s16* stream, int length);
}

#endif // __NBA_AUDIO_H__
