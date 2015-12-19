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

#include "gba_memory.h"
#include "common/log.h"

namespace NanoboyAdvance
{
    GBAMemory::GBAMemory(string bios_file, string rom_file)
    {
        bios = ReadFile(bios_file);
        rom = ReadFile(rom_file);
        gba_io = (GBAIO*)io;
        dma = new GBADMA(this, gba_io);
        timer = new GBATimer(gba_io);
        video = new GBAVideo(gba_io);
        memset(wram, 0, 0x40000);
        memset(iram, 0, 0x8000);
        memset(io, 0, 0x3FF);
        io[0x130] = 0xFF;
    }

    ubyte* GBAMemory::ReadFile(string filename)
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

    ubyte GBAMemory::ReadByte(uint offset)
    {
        int page = (offset >> 24) & 0xF;
        uint internal_offset = offset & 0xFFFFFF;
        bad_read = false;
        switch (page)
        {
        case 0:
            ASSERT(internal_offset >= 0x4000, LOG_ERROR, "BIOS read: offset out of bounds");
            if (internal_offset >= 0x4000)
            {
                return 0;
            }
            return bios[internal_offset];
        case 2:
            return wram[internal_offset % 0x40000];
        case 3:
            return iram[internal_offset % 0x8000];
        case 4:
            // Emulate IO mirror at 04xx0800
            // TODO: Fix?
            if ((internal_offset & 0xFFFF) == 0x800)
            {
                internal_offset &= 0xFFFF;
            }
            if (internal_offset >= 0x3FF)
            {
                LOG(LOG_ERROR, "IO read: offset out of bounds");
                return 0;
            }
            if (internal_offset < 0x200 && internal_offset > 0x203)
            LOG(LOG_INFO, "IO read: 0x%x", offset);
            return io[internal_offset];
        case 5:
            return video->pal[internal_offset % 0x400];
        case 6:
            internal_offset %= 0x20000;
            if (internal_offset >= 0x18000)
            {
                internal_offset -= 0x8000;
            }
            return video->vram[internal_offset];
        case 7:
            return video->obj[internal_offset % 0x400];
        case 8:
            // TODO: Prevent out of bounds read, we should save the rom size somewhere
            return rom[internal_offset];
        case 9:
            // TODO: Prevent out of bounds read, we should save the rom size somewhere
            return rom[0x1000000 + internal_offset];
        case 0xE:
            LOG(LOG_WARN, "Unhandled read from SRAM.");
            return 0;
        default:
            // Indicate to the debugger that we read a bad address
            bad_read = true;
            bad_address = offset;

            // Also log the error to the console
            LOG(LOG_ERROR, "Read from invalid/unimplemented address (0x%x)", offset);
            break;
        }
        return 0;
    }

    ushort GBAMemory::ReadHWord(uint offset)
    {
        // TODO: Handle special case SRAM
        return ReadByte(offset) | (ReadByte(offset + 1) << 8);
    }

    uint GBAMemory::ReadWord(uint offset)
    {
        // TODO: Handle special case SRAM
        return ReadByte(offset) | (ReadByte(offset + 1) << 8) | (ReadByte(offset + 2) << 16) | (ReadByte(offset + 3) << 24);
    }

    void GBAMemory::WriteByte(uint offset, ubyte value)
    {
        int page = (offset >> 24);
        uint internal_offset = offset & 0xFFFFFF;

        // Reset error indicator
        bad_write = false;

        // Perform write
        switch (page)
        {
        case 0:
            LOG(LOG_ERROR, "Write into BIOS memory not allowed (0x%x)", offset);
            break;
        case 2:
            wram[internal_offset % 0x40000] = value;
            break;
        case 3:
            iram[internal_offset % 0x8000] = value;
            break;
        case 4:
        {
            bool write = true;

            // If the address it out of bounds we should exit now
            if (internal_offset >= 0x3FF && (internal_offset & 0xFFFF) != 0x800)
            {
                LOG(LOG_ERROR, "IO write: offset out of bounds");
                break;
            }

            // Emulate IO mirror at 04xx0800
            if ((internal_offset & 0xFFFF) == 0x800)
            {
                internal_offset &= 0xFFFF;
            }

            // Writing to some registers causes special behaviour which must be emulated
            switch (internal_offset)
            {
            case DMA0CNT_H+1:
                if (value & (1 << 7))
                {
                    dma->dma0_source = gba_io->dma0sad;
                    dma->dma0_destination = gba_io->dma0dad;
                    dma->dma0_count = gba_io->dma0cnt_l;
                }
                break;
            case DMA1CNT_H + 1:
                if (value & (1 << 7))
                {
                    dma->dma1_source = gba_io->dma1sad;
                    dma->dma1_destination = gba_io->dma1dad;
                    dma->dma1_count = gba_io->dma1cnt_l;
                }
                break;
            case DMA2CNT_H + 1:
                if (value & (1 << 7))
                {
                    dma->dma2_source = gba_io->dma2sad;
                    dma->dma2_destination = gba_io->dma2dad;
                    dma->dma2_count = gba_io->dma2cnt_l;
                }
                break;
            case DMA3CNT_H + 1:
                if (value & (1 << 7))
                {
                    dma->dma3_source = gba_io->dma3sad;
                    dma->dma3_destination = gba_io->dma3dad;
                    dma->dma3_count = gba_io->dma3cnt_l;
                }
                break;
            case TM0CNT_L:
                timer->timer0_reload = (timer->timer0_reload & 0xFF00) | value;
                write = false;
                break;
            case TM0CNT_L+1:
                timer->timer0_reload = (timer->timer0_reload & 0x00FF) | (value << 8);
                write = false;
                break;
            case TM1CNT_L:
                timer->timer1_reload = (timer->timer1_reload & 0xFF00) | value;
                write = false;
                break;
            case TM1CNT_L+1:
                timer->timer1_reload = (timer->timer1_reload & 0x00FF) | (value << 8);
                write = false;
                break;
            case TM2CNT_L:
                timer->timer2_reload = (timer->timer2_reload & 0xFF00) | value;
                write = false;
                break;
            case TM2CNT_L+1:
                timer->timer2_reload = (timer->timer2_reload & 0x00FF) | (value << 8);
                write = false;
                break;
            case TM3CNT_L:
                timer->timer3_reload = (timer->timer3_reload & 0xFF00) | value;
                write = false;
                break;
            case TM3CNT_L+1:
                timer->timer3_reload = (timer->timer3_reload & 0x00FF) | (value << 8);
                write = false;
                break;
            case IF:
                gba_io->if_ &= ~value;
                write = false;
                break;
            }

            if (write)
            {
                io[internal_offset] = value;
            }
            break;
        }
        case 5:
        case 6:
        case 7:
            // We cannot write a single byte. Therefore the byte will be duplicated in the data bus and a halfword write will be performed
            WriteHWord(offset & ~1, (value << 8) | value);
            break;
        case 8:
        case 9:
            // Indicate to the debugger that we've a failure
            bad_write = true;
            bad_address = offset;
            bad_value = value;

            // Also log the error to the console
            LOG(LOG_ERROR, "Write into ROM memory not allowed (0x%x)", offset);
            break;
        case 0xE:
            LOG(LOG_WARN, "Unhandled write to SRAM.");
            break;
        default:
            // Indicate to the debugger that we've a failure
            bad_write = true;
            bad_address = offset;
            bad_value = value;

            // Also log the error to the console
            LOG(LOG_ERROR, "Write to invalid/unimplemented address (0x%x)", offset);
            break;
        }
    }

    void GBAMemory::WriteHWord(uint offset, ushort value)
    {
        int page = (offset >> 24) & 0xF;
        uint internal_offset = offset & 0xFFFFFF;
        switch (page)
        {
        case 5:
            video->pal[internal_offset % 0x400] = value & 0xFF;
            video->pal[(internal_offset + 1) % 0x400] = (value >> 8) & 0xFF;
            break;
        case 6:
            internal_offset %= 0x20000;
            if (internal_offset >= 0x18000)
            {
                internal_offset -= 0x8000;
            }
            video->vram[internal_offset] = value & 0xFF;
            video->vram[internal_offset + 1] = (value >> 8) & 0xFF;
            break;
        case 7:
            video->obj[internal_offset % 0x400] = value & 0xFF;
            video->obj[(internal_offset + 1) % 0x400] = (value >> 8) & 0xFF;
            break;
        default:
            WriteByte(offset, value & 0xFF);
            WriteByte(offset + 1, (value >> 8) & 0xFF);
            break;
        }
    }

    void GBAMemory::WriteWord(uint offset, uint value)
    {
        if (value & (1 << 23))
        {
            switch (offset)
            {
            case 0x04000100:
                gba_io->tm0cnt_l = value & 0xFFFF;
                break;
            case 0x04000104:
                gba_io->tm1cnt_l = value & 0xFFFF;
                break;
            case 0x04000108:
                gba_io->tm2cnt_l = value & 0xFFFF;
                break;
            case 0x0400010C:
                gba_io->tm3cnt_l = value & 0xFFFF;
                break;
            }
        }
        WriteHWord(offset, value & 0xFFFF);
        WriteHWord(offset + 2, (value >> 16) & 0xFFFF);
    }

}
