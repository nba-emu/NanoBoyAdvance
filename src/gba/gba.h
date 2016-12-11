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


#ifndef __NBA_GBA_H__
#define __NBA_GBA_H__


#include "arm/arm.hpp"
#include "memory.h"
#include "util/integer.hpp"
#include "util/file.hpp"
#include <string>


namespace GBA
{
    ///////////////////////////////////////////////////////////
    /// \file    gba.h
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \class   GBA
    /// \brief   The supervisor of all GBA-related classes.
    ///
    ///////////////////////////////////////////////////////////
    class GBA
    {
    public:
        enum class Key
        {
            None        = 0,
            A           = 1,
            B           = 2,
            Select      = 4,
            Start       = 8,
            Right       = 16,
            Left        = 32,
            Up          = 64,
            Down        = 128,
            R           = 256,
            L           = 512
        };

        GBA(std::string rom_file, std::string save_file);
        GBA(std::string rom_file, std::string save_file, std::string bios_file);
        ~GBA();

        void Frame();
        void SetKeyState(Key key, bool pressed);
        void SetSpeedUp(int multiplier);
        u32* GetVideoBuffer();
        bool HasRendered();

    private:

        static const int FRAME_CYCLES;

        arm m_ARM;
        Memory m_Memory;
        int m_SpeedMultiplier        {1};      ///< Holds the emulation speed
        bool m_DidRender             {false};  ///< Has frame already been rendered?
    };
}


#endif  // __NBA_GBA_H__
