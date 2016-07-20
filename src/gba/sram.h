/*
* Copyright (C) 2016 Frederic Meyer
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

#include "backup.h"
#include <string>

#pragma once

namespace NanoboyAdvance
{
    class SRAM : public GBABackup
    {
        static const int SAVE_SIZE;

        std::string save_file;
        u8 memory[65536]; 
    public:
        // Constructor and Destructor
        SRAM(std::string save_file);
        ~SRAM();

        // Read and write access
        u8 ReadByte(u32 offset);
        void WriteByte(u32 offset, u8 value);
    };
};
