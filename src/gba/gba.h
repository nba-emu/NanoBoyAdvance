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


#include "arm.h"
#include "memory.h"
#include "soft_composer.h"
#include "util/types.h"
#include "util/file.h"
#include <string>


namespace NanoboyAdvance
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

        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   Key
        ///
        /// Bitfield for all GBA keys (0x3FF)
        ///
        ///////////////////////////////////////////////////////////
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


        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Constructor, 1
        ///
        ///////////////////////////////////////////////////////////
        GBA(std::string rom_file, std::string save_file);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Constructor, 2
        ///
        ///////////////////////////////////////////////////////////
        GBA(std::string rom_file, std::string save_file, std::string bios_file);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Frame
        /// \brief   Executes one frame within the GBA.
        ///
        ///////////////////////////////////////////////////////////
        void Frame();

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      SetKeyState
        /// \brief   Sets the key either pressed or released.
        ///
        /// \param   key      The key being updated.
        /// \param   pressed  True for pressed.
        ///
        ///////////////////////////////////////////////////////////
        void SetKeyState(Key key, bool pressed);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      SetSpeedUp
        /// \brief   Changes the emulation speed.
        ///
        /// \param  multiplier  The multiple of emulation speed.
        ///
        ///////////////////////////////////////////////////////////
        void SetSpeedUp(int multiplier);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      GetVideoBuffer
        /// \brief   Retrieves the video pixel buffer.
        ///
        ///////////////////////////////////////////////////////////
        u32* GetVideoBuffer();

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      HasRendered
        /// \brief   Determines whether the frame was rendered.
        ///
        ///////////////////////////////////////////////////////////
        bool HasRendered();


    private:

        static const int FRAME_CYCLES;

        ///////////////////////////////////////////////////////////
        // Class members (gba slaves)
        //
        ///////////////////////////////////////////////////////////
        ARM7 m_ARM;             ///< Processor instance
        Memory m_Memory;        ///< Memory instance


        ///////////////////////////////////////////////////////////
        // Class members (misc)
        //
        ///////////////////////////////////////////////////////////
        int m_SpeedMultiplier        {1};      ///< Holds the emulation speed
        bool m_DidRender             {false};  ///< Has frame already been rendered?
        GBASoftComposer m_Composer;            ///< Final picture composer
    };
}


#endif  // __NBA_GBA_H__
