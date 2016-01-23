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
#include <fstream>
#include "common/types.h"
#include "memory.h"
#include "gba_io.h"
#include "gba_dma.h"
#include "gba_timer.h"
#include "gba_video.h"

using namespace std;

// TODO: Using C-style callbacks in C++ is bad practice I guess..
typedef void (*MemoryCallback)(u32 address, int size, bool write, bool invalid);

namespace NanoboyAdvance
{
    class GBAMemory : public Memory
    {
        u8* bios;
        u8 wram[0x40000];
        u8 iram[0x8000];
        u8 io[0x3FF];
        u8* rom;
        u8 sram[0x10000];
        MemoryCallback memory_hook;
        static u8* ReadFile(string filename);
    public:
        // Hardware / IO accessible through memory
        GBAIO* gba_io;
        GBADMA* dma;
        GBATimer* timer;
        GBAVideo* video;
 
        // Sets an callback that gets called each time memory is accessed (unimpemented)
        void SetCallback(MemoryCallback callback);

        // Read / Write access methods
        u8 ReadByte(u32 offset);
        u16 ReadHWord(u32 offset);
        u32 ReadWord(u32 offset);
        void WriteByte(u32 offset, u8 value);
        void WriteHWord(u32 offset, u16 value);
        void WriteWord(u32 offset, u32 value);

        // Constructor
        GBAMemory(string bios_file, string rom_file);
    };
}
