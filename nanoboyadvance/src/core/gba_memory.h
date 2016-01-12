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
typedef void (*MemoryCallback)(uint address, int size, bool write, bool invalid);

namespace NanoboyAdvance
{
    class GBAMemory : public Memory
    {
        ubyte* bios;
        ubyte wram[0x40000];
        ubyte iram[0x8000];
        ubyte io[0x3FF];
        ubyte* rom;
        MemoryCallback memory_hook;
        static ubyte* ReadFile(string filename);
    public:
        GBAIO* gba_io;
        GBADMA* dma;
        GBATimer* timer;
        GBAVideo* video;
        void SetCallback(MemoryCallback callback);
        ubyte ReadByte(uint offset);
        ushort ReadHWord(uint offset);
        uint ReadWord(uint offset);
        void WriteByte(uint offset, ubyte value);
        void WriteHWord(uint offset, ushort value);
        void WriteWord(uint offset, uint value);
        GBAMemory(string bios_file, string rom_file);
    };
}
