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
#include "../common/log.h"

using namespace std;

namespace NanoboyAdvance
{
	GBAMemory::GBAMemory(string bios_file, string rom_file)
	{
		bios = ReadFile(bios_file);
		rom = ReadFile(rom_file);
		gba_io = (GBAIO*)io;
		dma = new GBADMA { this, gba_io };
		timer = new GBATimer { gba_io };
		video = new GBAVideo { gba_io };
		memset(wram, 0, 0x40000);
		memset(iram, 0, 0x8000);
		memset(io, 0, 0x3FF);
		io[0x130] = 0xFF;
	}

	ubyte* GBAMemory::ReadFile(string filename)
	{
		ifstream ifs(filename, ios::in | ios::binary | ios::ate);
		size_t filesize { };
		ubyte* data = 0;
		if (ifs.is_open())
		{
			ifs.seekg(0, ios::end);
			filesize = static_cast<size_t>(ifs.tellg());
			ifs.seekg(0, ios::beg);
			data = new ubyte[filesize] { };
			ifs.read((char*)data, filesize);
		}
		else
		{
			cout << "Cannot open file " << filename.c_str();
			return nullptr;
		}
		return data;
	}

	ubyte GBAMemory::ReadByte(uint offset)
	{
		int page = (offset >> 24) & 0xF;
		uint internal_offset = offset & 0xFFFFFF;
		switch (page)
		{
			case 0:
			ASSERT(internal_offset >= 0x4000, LOG_ERROR, "BIOS read: offset out of bounds");
				return bios[internal_offset];
			case 2:
				//ASSERT(internal_offset >= 0x40000, LOG_ERROR, "WRAM read: offset out of bounds");
				return wram[internal_offset % 0x40000];
			case 3:
				//ASSERT(internal_offset >= 0x8000 && internal_offset <= 0xFFFF00, LOG_ERROR, "IRAM read: offset out of bounds");
				return iram[internal_offset % 0x80000];
			case 4:
				// Emulate IO mirror at 04xx0800
				if ((internal_offset & 0xFFFF) == 0x800)
				{
					internal_offset &= 0xFFFF;
				}
				ASSERT(internal_offset >= 0x3FF, LOG_ERROR, "IO read: offset out of bounds");
				return io[internal_offset];
			case 5:
				//ASSERT(internal_offset >= 0x400, LOG_ERROR, "PAL read: offset out of bounds");
				return video->pal[internal_offset % 0x400];
			case 6:
				//ASSERT(internal_offset >= 0x18000, LOG_ERROR, "VRAM read: offset out of bounds");
				internal_offset %= 0x20000;
				if (internal_offset >= 0x18000)
				{
					internal_offset -= 0x8000;
				}
				return video->vram[internal_offset];
			case 7:
				//ASSERT(internal_offset >= 0x400, LOG_ERROR, "OAM read: offset out of bounds");
				return video->obj[internal_offset % 0x400];
			case 8:
				// TODO: Prevent out of bounds read, we should save the rom size somewhere
				return rom[internal_offset];
			case 9:
				// TODO: Prevent out of bounds read, we should save the rom size somewhere
				return rom[0x1000000 + internal_offset];
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
		int page = (offset >> 24) & 0xF;
		uint internal_offset = offset & 0xFFFFFF;
		switch (page)
		{
			case 0:
				//LOG(LOG_ERROR, "Write into BIOS memory not allowed (0x%x)", offset);
				break;
			case 2:
				//ASSERT(internal_offset >= 0x40000, LOG_ERROR, "WRAM write: offset out of bounds");
				wram[internal_offset % 0x40000] = value;
				break;
			case 3:
				//ASSERT(internal_offset >= 0x8000 && internal_offset <= 0xFFFF00, LOG_ERROR, "IRAM write: offset out of bounds");
				iram[internal_offset % 0x80000] = value;
				break;
			case 4:
			{
				bool write { true };
				ASSERT(internal_offset >= 0x3FF, LOG_ERROR, "IO write: offset out of bounds");

				// If the address it out of bounds we should exit now
				if (internal_offset >= 0x3FF)
				{
					break;
				}

				// Emulate IO mirror 04xx0800
				if ((internal_offset % 0xFFFF) == 0x800)
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
							LOG(LOG_INFO, "DMA0 source=0x%x dest=0x%x count=0x%x", dma->dma0_source, dma->dma0_destination, dma->dma0_count);
						}
						break;
					case DMA1CNT_H + 1:
						if (value & (1 << 7))
						{
							dma->dma1_source = gba_io->dma1sad;
							dma->dma1_destination = gba_io->dma1dad;
							dma->dma1_count = gba_io->dma1cnt_l;
							LOG(LOG_INFO, "DMA1 source=0x%x dest=0x%x count=0x%x", dma->dma1_source, dma->dma1_destination, dma->dma1_count);
						}
						break;
					case DMA2CNT_H + 1:
						if (value & (1 << 7))
						{
							dma->dma2_source = gba_io->dma2sad;
							dma->dma2_destination = gba_io->dma2dad;
							dma->dma2_count = gba_io->dma2cnt_l;
							LOG(LOG_INFO, "DMA2 source=0x%x dest=0x%x count=0x%x", dma->dma2_source, dma->dma2_destination, dma->dma2_count);
						}
						break;
					case DMA3CNT_H + 1:
						if (value & (1 << 7))
						{
							dma->dma3_source = gba_io->dma3sad;
							dma->dma3_destination = gba_io->dma3dad;
							dma->dma3_count = gba_io->dma3cnt_l;
							LOG(LOG_INFO, "DMA3 source=0x%x dest=0x%x count=0x%x", dma->dma3_source, dma->dma3_destination, dma->dma3_count);
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
			LOG(LOG_ERROR, "Write into ROM memory not allowed (0x%x)", offset);
				break;
			default:
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
				//ASSERT(internal_offset + 1 >= 0x400, LOG_ERROR, "PAL write: offset out of bounds");
				video->pal[internal_offset % 0x400] = value & 0xFF;
				video->pal[(internal_offset + 1) % 0x400] = (value >> 8) & 0xFF;
				break;
			case 6:
				//ASSERT(internal_offset + 1 >= 0x18000, LOG_ERROR, "VRAM write: offset out of bounds");
				internal_offset %= 0x20000;
				if (internal_offset >= 0x18000)
				{
					internal_offset -= 0x8000;
				}
				video->vram[internal_offset] = value & 0xFF;
				video->vram[internal_offset + 1] = (value >> 8) & 0xFF;
				break;
			case 7:
				//ASSERT(internal_offset + 1 >= 0x400, LOG_ERROR, "OAM write: offset out of bounds");
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
		//ASSERT(offset == 0x4000100 || offset == 0x4000104 || offset == 0x4000108 || offset == 0x400010C, LOG_WARN, "Unimplemented case 32 bit write to timer register");
		WriteHWord(offset, value & 0xFFFF);
		WriteHWord(offset + 2, (value >> 16) & 0xFFFF);
	}

}
