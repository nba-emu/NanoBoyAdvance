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


#ifndef __NBA_FLASH_H__
#define __NBA_FLASH_H__


#include "backup.h"
#include <string>


namespace NanoboyAdvance
{
    ///////////////////////////////////////////////////////////
    /// \file    flash.h
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \class   GBAFlash
    /// \brief   Emulates FLASH64/128 backup memory.
    ///
    ///////////////////////////////////////////////////////////
    class GBAFlash : public GBABackup
    {
    private:
        ///////////////////////////////////////////////////////////
        /// \author Frederic Meyer
        /// \date   July 31th, 2016
        /// \enum   FlashCommand
        ///
        /// Defines all possible FLASH commands.
        ///
        ///////////////////////////////////////////////////////////
        enum class FlashCommand
        {
            READ_CHIP_ID        = 0x90,
            FINISH_CHIP_ID      = 0xF0,
            ERASE               = 0x80,
            ERASE_CHIP          = 0x10,
            ERASE_SECTOR        = 0x30,
            WRITE_BYTE          = 0xA0,
            SELECT_BANK         = 0xB0
        };


    public:

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Constructor
        /// \param   save_file        Path to the underlying save file.
        /// \param   second_bank      True for FLASH128.
        ///
        ///////////////////////////////////////////////////////////
        GBAFlash(std::string save_file, bool second_bank);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      Destructor
        ///
        ///////////////////////////////////////////////////////////
        ~GBAFlash();


        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      ReadByte
        ///
        /// Reads a byte at specified offset of the flash memory.
        ///
        /// \param   offset  FLASH address to read from.
        /// \return  The read value.
        ///
        ///////////////////////////////////////////////////////////
        u8 ReadByte(u32 offset);

        ///////////////////////////////////////////////////////////
        /// \author  Frederic Meyer
        /// \date    July 31th, 2016
        /// \fn      WriteByte
        ///
        /// Writes a byte at specified offset of the flash memory.
        ///
        /// \param  offset  FLASH address to write to.
        /// \param  value   Value to write to the bus.
        ///////////////////////////////////////////////////////////
        void WriteByte(u32 offset, u8 value);


    private:

        ///////////////////////////////////////////////////////////////////////////////////
        // Class members (memory)
        //
        ///////////////////////////////////////////////////////////////////////////////////
        std::string m_SaveFile      { "" };         ///< Path to the save file
        bool m_SecondBank           { false };      ///< Use a second memory bank?
        u8 m_Memory                 [2][65536];     ///< Defines two memory banks with 64kB each
        int m_MemoryBank            { 0 };          ///< Defines the current memory bank


        ///////////////////////////////////////////////////////////////////////////////////
        // Class members (command)
        //
        ///////////////////////////////////////////////////////////////////////////////////
        int m_CommandPhase          { 0 };          ///< Holds the current phase of the command
        bool m_EnableChipID         { false };      ///< Allow to read chip identifier?
        bool m_EnableErase          { false };      ///< Enable erasing bytes from the chip?
        bool m_EnableByteWrite      { false };      ///< Enable writing bytes to the chip?
        bool m_EnableBankSelect     { false };      ///< Allow selecting a specific bank?
    };
}


#endif  // __NBA_FLASH_H__

