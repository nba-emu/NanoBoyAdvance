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

#include "gba_backup_flash.h"
#include "common/log.h"
#include "common/file.h"

using namespace std;

namespace NanoboyAdvance
{
    GBAFlash::GBAFlash(string save_file, bool second_bank)
    {
        u8* save_data;

        // Save save file path and flash type
        this->save_file = save_file;        
        this->second_bank = second_bank;

        // Check if save file already exists, sanitize and load if so        
        if (File::Exists(save_file))
        {
            int size = File::GetFileSize(save_file);
            if (size == (second_bank ? 131072 : 65536))
            {
                save_data = File::ReadFile(save_file);
                for (int i = 0; i < 65536; i++)
                {
                    memory[0][i] = save_data[i];
                    memory[1][i] = second_bank ? save_data[65536 + i] : 0;
                }
                return;
            }
            else { LOG(LOG_ERROR, "Save file size is invalid"); }
        }

        // If loading a save file failed, init save with FFh, just like if the chip was erased
        for (int i = 0; i < 65536; i++)
        {    
            memory[0][i] = 0xFF;
            memory[1][i] = 0xFF;
        }
    }

    GBAFlash::~GBAFlash()
    {
        if (second_bank)
        {
            u8 buffer[131072];
            for (int i = 0; i < 65536; i++)
            {
                buffer[i] = memory[0][i];
                buffer[65536 + i] = memory[1][i];
            }
            File::WriteFile(save_file, buffer, 131072);
        }
        else
        {
            File::WriteFile(save_file, memory[0], 65536);
        }
    }   

    u8 GBAFlash::ReadByte(u32 offset)
    {
        offset &= 0xFFFF;
        if (enable_chip_id && offset < 2)
        {
            // Chip identifier for FLASH64: D4BF (SST 64K)
            // Chip identifier for FLASH128: 09C2 (Macronix 128K)
            if (offset == 0 && second_bank) return 0xC2;
            else if (offset == 0) return 0xBF;
            else if (second_bank) return 0x09;
            else return 0xD4;
        }
        return memory[memory_bank][offset];
    }

    void GBAFlash::WriteByte(u32 offset, u8 value)
    {
        // todo: if single byte write is pending, prioritize it before command initiation
        if (offset == 0x0E005555 && value == 0xAA) { command_phase = 1; LOG(LOG_INFO, " Phase 1"); }
        else if (offset == 0x0E002AAA && value == 0x55 && command_phase == 1) { command_phase = 2; LOG(LOG_INFO, "Phase 2"); }
        else if (offset == 0x0E005555 && command_phase == 2) 
        {
            LOG(LOG_INFO, "Interpreting command...");
            // interpret command
            switch (static_cast<GBAFlashCommand>(value)) 
            {
            case GBAFlashCommand::READ_CHIP_ID: enable_chip_id = true; LOG(LOG_INFO, "Enabling chip id"); break;
            case GBAFlashCommand::FINISH_CHIP_ID: enable_chip_id = false; LOG(LOG_INFO, "Disabling chip id"); break;
            case GBAFlashCommand::ERASE: enable_erase = true; break;
            case GBAFlashCommand::ERASE_CHIP: 
                if (enable_erase)
                {
                    for (int i = 0; i < 65536; i++) // loop is propably more efficient than calling memset twice
                    {
                        memory[0][i] = 0xFF;
                        memory[1][i] = 0xFF; // do it even if chip is FLASH64, we spare an extra branch
                    }
                    enable_erase = false;
                } 
                break;
            case GBAFlashCommand::WRITE_BYTE: enable_byte_write = true;
            case GBAFlashCommand::SELECT_BANK: if (second_bank) enable_bank_select = true;
            }
            command_phase = 0;
        }
        else if (enable_erase && static_cast<GBAFlashCommand>(value) == GBAFlashCommand::ERASE_SECTOR && (
                     offset & ~0xF000) == 0x0E000000)
        {
            int base_offset = offset & 0xF000;
            for (int i = 0; i < 0x1000; i++)
            {
                memory[memory_bank][base_offset + i] = 0xFF;
            }
            enable_erase = false;
        }
        // todo: allow single byte write only if affected sector was erased beforehand.
        else if (enable_byte_write) { memory[memory_bank][offset & 0xFFFF] = value; enable_byte_write = false; } 
        else if (enable_bank_select && offset == 0x0E000000) { memory_bank = value & 1; enable_bank_select = false;  }
    }
}
