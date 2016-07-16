/*
* Copyright (C) 2015 Frederic Meyer
*
* This file is part of nanoboyadvance.
*
* nanoboyadvance is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* nanoboyadvance is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
*/

#include <string>
#include "backup.h"

#pragma once

namespace NanoboyAdvance
{
    class GBAFlash : public GBABackup
    {
        enum class FlashCommand
        {
            READ_CHIP_ID = 0x90,
            FINISH_CHIP_ID = 0xF0,
            ERASE = 0x80,
            ERASE_CHIP = 0x10,
            ERASE_SECTOR = 0x30,
            WRITE_BYTE = 0xA0,
            SELECT_BANK = 0xB0
        };

        // Path to the save file
        std::string save_file;

        // Memory
        bool second_bank { false };
        u8 memory[2][65536];
        int memory_bank { 0 };  

        // Command processing
        int command_phase { 0 };
        bool enable_chip_id { false };
        bool enable_erase { false };
        bool enable_byte_write { false };
        bool enable_bank_select { false };
    public:
        // Constructor and Destructor
        GBAFlash(std::string save_file, bool second_bank);
        ~GBAFlash();

        // Read and write access
        u8 ReadByte(u32 offset);
        void WriteByte(u32 offset, u8 value);
    };
};
