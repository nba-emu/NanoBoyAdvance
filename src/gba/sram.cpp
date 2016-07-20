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

#include "sram.h"
#include "util/log.h"
#include "util/file.h"
#include <cstring>

using namespace std;

namespace NanoboyAdvance
{
    const int SRAM::SAVE_SIZE = 65536;

    SRAM::SRAM(string save_file)
    {
        u8* save_data;

        // Save save file path
        this->save_file = save_file;        

        // Check if save file already exists, sanitize and load if so        
        if (File::Exists(save_file))
        {
            int size = File::GetFileSize(save_file);

            if (size == SAVE_SIZE)
            {
                save_data = File::ReadFile(save_file);
                memcpy(memory, save_data, SAVE_SIZE);
                return;
            }
            else { LOG(LOG_ERROR, "Save file size is invalid"); }
        }

        // Zero-init if loading failed
        memset(memory, 0, SAVE_SIZE);
    }

    SRAM::~SRAM()
    {
        File::WriteFile(save_file, memory, SAVE_SIZE);
    }   

    u8 SRAM::ReadByte(u32 offset)
    {
        return memory[offset % SAVE_SIZE];
    }

    void SRAM::WriteByte(u32 offset, u8 value)
    {
        memory[offset % SAVE_SIZE] = value;
    }
}
