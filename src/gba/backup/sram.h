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


#ifndef __NBA_SRAM_H__
#define __NBA_SRAM_H__


#include "backup.h"
#include <string>


namespace GBA
{
    ///////////////////////////////////////////////////////////
    /// \file    sram.h
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \class   SRAM
    /// \brief   Defines the SRAM memory save type.
    ///
    ///////////////////////////////////////////////////////////
    class SRAM : public Backup
    {
        static const int SAVE_SIZE;


    public:

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Constructor
        ///
        ///////////////////////////////////////////////////////////
        SRAM(std::string m_SaveFile);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Destructor
        ///
        ///////////////////////////////////////////////////////////
        ~SRAM();


        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      ReadByte
        ///
        /// Reads one byte from SRAM memory.
        ///
        /// \param  offset  The address to read from.
        ///
        ///////////////////////////////////////////////////////////
        u8 ReadByte(u32 offset);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      WriteByte
        ///
        /// Writes one byte into SRAM memory.
        ///
        /// \param  offset  The address to write to.
        /// \param  value   The value being written.
        ///
        ///////////////////////////////////////////////////////////
        void WriteByte(u32 offset, u8 value);


    private:

        ///////////////////////////////////////////////////////////
        // Class members
        //
        ///////////////////////////////////////////////////////////
        std::string m_SaveFile;
        u8 m_Memory[65536];
    };
}


#endif  // __NBA_SRAM_H__
