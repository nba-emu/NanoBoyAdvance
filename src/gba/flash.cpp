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


#include "flash.h"
#include "common/log.h"
#include "common/file.h"


using namespace std;
using namespace Common;


namespace NanoboyAdvance
{
    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      Constructor
    ///
    ///////////////////////////////////////////////////////////
    Flash::Flash(string save_file, bool second_bank)
    {
        u8* save_data;

        // Save save file path and flash type
        m_SaveFile = save_file;
        m_SecondBank = second_bank;

        // Check if save file already exists, sanitize and load if so        
        if (File::Exists(save_file))
        {
            int size = File::GetFileSize(save_file);
            if (size == (second_bank ? 131072 : 65536))
            {
                save_data = File::ReadFile(save_file);
                for (int i = 0; i < 65536; i++)
                {
                    m_Memory[0][i] = save_data[i];
                    m_Memory[1][i] = second_bank ? save_data[65536 + i] : 0;
                }
                return;
            }
            else { LOG(LOG_ERROR, "Save file size is invalid"); }
        }

        // If loading a save file failed, init save with FFh, just like if the chip was erased
        for (int i = 0; i < 65536; i++)
        {    
            m_Memory[0][i] = 0xFF;
            m_Memory[1][i] = 0xFF;
        }
    }

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      Destructor
    ///
    ///////////////////////////////////////////////////////////
    Flash::~Flash()
    {
        if (m_SecondBank)
        {
            u8 buffer[131072];
            for (int i = 0; i < 65536; i++)
            {
                buffer[i] = m_Memory[0][i];
                buffer[65536 + i] = m_Memory[1][i];
            }
            File::WriteFile(m_SaveFile, buffer, 131072);
        }
        else
        {
            File::WriteFile(m_SaveFile, m_Memory[0], 65536);
        }
    }   


    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      ReadByte
    ///
    ///////////////////////////////////////////////////////////
    u8 Flash::ReadByte(u32 offset)
    {
        offset &= 0xFFFF;
        // TODO: vba-sdl-h source codes suggests chip id being mirrored each 100h bytes
        // however gbatek doesn't mention this (?)        
        if (m_EnableChipID && offset < 2)
        {
            // Chip identifier for FLASH64: D4BF (SST 64K)
            // Chip identifier for FLASH128: 09C2 (Macronix 128K)
            if (offset == 0 && m_SecondBank) return 0xC2;
            else if (offset == 0) return 0xBF;
            else if (m_SecondBank) return 0x09;
            else return 0xD4;
        }
        return m_Memory[m_MemoryBank][offset];
    }

    ///////////////////////////////////////////////////////////
    /// \author  Frederic Meyer
    /// \date    July 31th, 2016
    /// \fn      WriteByte
    ///
    ///////////////////////////////////////////////////////////
    void Flash::WriteByte(u32 offset, u8 value)
    {
        if (!m_EnableByteWrite && offset == 0x0E005555 && value == 0xAA) { m_CommandPhase = 1; }
        else if (offset == 0x0E002AAA && value == 0x55 && m_CommandPhase == 1) { m_CommandPhase = 2; }
        else if (offset == 0x0E005555 && m_CommandPhase == 2)
        {
            // Interpret command
            switch (static_cast<FlashCommand>(value)) 
            {
            case FlashCommand::READ_CHIP_ID: m_EnableChipID = true; break;
            case FlashCommand::FINISH_CHIP_ID: m_EnableChipID = false; break;
            case FlashCommand::ERASE: m_EnableErase = true; break;
            case FlashCommand::ERASE_CHIP: 
                if (m_EnableErase)
                {
                    for (int i = 0; i < 65536; i++)
                    {
                        m_Memory[0][i] = 0xFF;
                        if (m_SecondBank)
                            m_Memory[1][i] = 0xFF;
                    }
                    m_EnableErase = false;
                } 
                break;
            case FlashCommand::WRITE_BYTE: m_EnableByteWrite = true;
            case FlashCommand::SELECT_BANK: if (m_SecondBank) m_EnableBankSelect = true;
            }
            m_CommandPhase = 0;
        }
        else if (m_EnableErase && static_cast<FlashCommand>(value) == FlashCommand::ERASE_SECTOR && (
                 offset & ~0xF000) == 0x0E000000 && m_CommandPhase == 2)
        {
            int base_offset = offset & 0xF000;

            for (int i = 0; i < 0x1000; i++)
                m_Memory[m_MemoryBank][base_offset + i] = 0xFF;

            m_EnableErase = false;
            m_CommandPhase = 0;
        }
        else if (m_EnableByteWrite)
        { 
            m_Memory[m_MemoryBank][offset & 0xFFFF] = value;
            m_EnableByteWrite = false;
        } 
        else if (m_EnableBankSelect && offset == 0x0E000000)
        {
            m_MemoryBank = value & 1;
            m_EnableBankSelect = false;
        }
    }
}
