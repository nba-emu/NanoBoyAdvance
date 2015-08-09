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
#include "common/log.h"

namespace NanoboyAdvance
{
    // TODO: Remember that ROMs over 16MB reach from 08.. to 09.. area.

    PagedMemory::PagedMemory()
    {
        for (int i = 0; i < 0x40000; i++) wram[i] = 0;
        for (int i = 0; i < 0x8000; i++) iram[i] = 0;
        for (int i = 0; i < 0x400; i++) pram[i] = 0;
        bios = PagedMemory::ReadFile("bios.bin");
        rom = PagedMemory::ReadFile("rom.gba");
    }

    ubyte PagedMemory::ReadByte(uint offset)
    {
        int page = offset >> 24;
        uint internal_offset = offset & 0xFFFFFF;
        switch (page)
        {
        case 0:
            // BIOS
            return bios[internal_offset];
        default:
            LOG(LOG_WARN, "Read from unimplemented 0x%x", offset);
            break;
        }
        return 0;
    }
    
    ushort PagedMemory::ReadHWord(uint offset)
    {
        uint real_offset = offset & ~1;
        ushort result;
        result = ReadByte(real_offset);
        result |= ReadByte(real_offset + 1) << 8;
        return result;
    }
    
    uint PagedMemory::ReadWord(uint offset)
    {
        uint real_offset = offset & ~3;
        uint result;
        result = ReadByte(real_offset);
        result |= ReadByte(real_offset + 1) << 8;
        result |= ReadByte(real_offset + 2) << 16;
        result |= ReadByte(real_offset + 3) << 24;
        return result;
    }

    void PagedMemory::WriteByte(uint offset, ubyte value)
    {
    }

    void PagedMemory::WriteHWord(uint offset, ushort value)
    {
    }

    void PagedMemory::WriteWord(uint offset, uint value)
    {
    }

    ubyte* PagedMemory::ReadFile(string filename)
    {
        ifstream ifs(filename, ios::in | ios::binary | ios::ate);
        size_t filesize;
        ubyte* data = 0;
        if (ifs.is_open())
        {
            ifs.seekg(0, ios::end);
            filesize = ifs.tellg();
            ifs.seekg(0, ios::beg);
            data = new ubyte[filesize];
            ifs.read((char*)data, filesize);
        }
        else
        {
            cout << "Cannot open file " << filename.c_str();
            return NULL;
        }
        return data;
    }

} 