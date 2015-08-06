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

#include "memory.h"

namespace NanoboyAdvance
{
    // TODO: Remember that ROMs over 16MB reach from 08.. to 09.. area.
    ubyte PagedMemory::ReadByte(uint offset)
    {
        MemoryPage page = Pages[offset >> 24];
        ubyte result = page.ReadByte(offset & 0xFFFFFF);
        Cycles += page.Cycles8;
        return result;
    }
    
    ushort PagedMemory::ReadHWord(uint offset)
    {
        MemoryPage page = Pages[offset >> 24];
        ushort result = page.ReadHWord(offset & 0xFFFFFF);
        Cycles += page.Cycles16;
        return result;
    }
    
    uint PagedMemory::ReadWord(uint offset)
    {
        MemoryPage page = Pages[offset >> 24];
        uint result = page.ReadWord(offset & 0xFFFFFF);
        Cycles += page.Cycles32;
        return result;
    }

    void PagedMemory::WriteByte(uint offset, ubyte value)
    {
        MemoryPage page = Pages[offset >> 24];
        page.WriteByte(offset & 0xFFFFFF, value);
        Cycles += page.Cycles8;
    }

    void PagedMemory::WriteHWord(uint offset, ushort value)
    {
        MemoryPage page = Pages[offset >> 24];
        page.WriteHWord(offset & 0xFFFFFF, value);
        Cycles += page.Cycles16;
    }

    void PagedMemory::WriteWord(uint offset, uint value)
    {
        MemoryPage page = Pages[offset >> 24];
        page.WriteWord(offset & 0xFFFFFF, value);
        Cycles += page.Cycles32;
    }
} 