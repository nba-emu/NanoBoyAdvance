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

#pragma once

#include <iostream>
#include "common/types.h"
#include "gba_io.h"
#include "gba_timer.h"
#include "gba_video.h"
#include "gba_backup_flash.h"

using namespace std;

// TODO: Using C-style callbacks in C++ is bad practice I guess..
typedef void (*MemoryCallback)(u32 address, bool write, bool invalid);

namespace NanoboyAdvance
{
    class GBAMemory
    {
        u8* bios;
        u8 wram[0x40000];
        u8 iram[0x8000];
        u8 io[0x3FF];
        u8* rom;
        int rom_size;
        u8 sram[0x10000];      
        GBABackup* backup;
        
        // DMA related
        enum AddressControl
        {
            Increment = 0,
            Decrement = 1,
            Fixed = 2,
            IncrementAndReload = 3
        };
        
        // DMA (internal) IO
        u32 dma_src[4];
        u32 dma_dst[4];
        u32 dma_cnt[4];
        u16* dma_cntl[4];
        
        // Debugging related callback
        MemoryCallback memory_hook;
        
        // Pointer-safe call to memory_hook (avoid nullpointer)
        inline void MemoryHook(u32 address, bool write, bool invalid)
        {
            if (memory_hook != NULL)
            {
                memory_hook(address, write, invalid);
            }
        }
    public:
        // Hardware / IO accessible through memory
        GBAIO* gba_io;
        GBATimer* timer;
        GBAVideo* video;

        // Flags
        enum class GBAHaltState
        {
            None,
            Stop,
            Halt
        };
        GBAHaltState halt_state { GBAHaltState::None };
        bool intr_wait { false };
        bool intr_wait_mask { 0 };

        enum class GBASaveType
        {
            EEPROM,
            SRAM,
            FLASH64,
            FLASH128
        };
        GBASaveType save_type { GBASaveType::SRAM };

        // Sets an callback that gets called each time memory is accessed (unimpemented)
        void SetCallback(MemoryCallback callback);

        // Schedules DMA
        void Step();
        
        // Read / Write access methods
        u8 ReadByte(u32 offset);
        u16 ReadHWord(u32 offset);
        u32 ReadWord(u32 offset);
        void WriteByte(u32 offset, u8 value);
        void WriteHWord(u32 offset, u16 value);
        void WriteWord(u32 offset, u32 value);
        
        // Constructor and Destructor
        GBAMemory(string bios_file, string rom_file, string save_file);
        ~GBAMemory();
    };
}
