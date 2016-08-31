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


#include "adapter.h"


namespace NanoboyAdvance
{
    ///////////////////////////////////////////////////////////
    /// \file    sdl_adapter.h
    /// \class   SDL2AudioAdapter
    /// \brief   Provides audio output per SDL2_Audio.
    ///
    ///////////////////////////////////////////////////////////
    class SDL2AudioAdapter : public AudioAdapter
    {
    public:
        void Init(Audio* audio);
        void Deinit();
        void Pause();
        void Resume();
    };
}
