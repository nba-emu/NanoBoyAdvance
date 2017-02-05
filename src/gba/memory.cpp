///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2017 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////

#include "cpu.hpp"
#include "util/logger.hpp"

using namespace util;

namespace gba
{
    u8 cpu::bus_read_byte(u32 address)
    {
        register int page = (address >> 24) & 15;
        register read_func func = m_read_table[page];

        m_cycles -= m_mem_cycles8_16[page];

        return (this->*func)(address);
    }

    u16 cpu::bus_read_hword(u32 address)
    {
        register int page = (address >> 24) & 15;
        register read_func func = m_read_table[page];

        m_cycles -= m_mem_cycles8_16[page];

        return (this->*func)(address) | ((this->*func)(address + 1) << 8);
    }

    u32 cpu::bus_read_word(u32 address)
    {
        register int page = (address >> 24) & 15;
        register read_func func = m_read_table[page];

        m_cycles -= m_mem_cycles32[page];

        return (this->*func)(address) |
               ((this->*func)(address + 1) << 8) |
               ((this->*func)(address + 2) << 16) |
               ((this->*func)(address + 3) << 24);
    }

    void cpu::bus_write_byte(u32 address, u8 value)
    {
        register int page = (address >> 24) & 15;
        register write_func func = m_write_table[page];

        m_cycles -= m_mem_cycles8_16[page];

        // handle 16-bit data busses. might reduce condition?
        if (page == 5 || page == 6 || page == 7)
        {
            (this->*func)(address & ~1, value);
            (this->*func)((address & ~1) + 1, value);
            return;
        }

        (this->*func)(address, value);
    }

    void cpu::bus_write_hword(u32 address, u16 value)
    {
        register int page = (address >> 24) & 15;
        register write_func func = m_write_table[page];

        m_cycles -= m_mem_cycles8_16[page];

        (this->*func)(address, value & 0xFF);
        (this->*func)(address + 1, value >> 8);
    }

    void cpu::bus_write_word(u32 address, u32 value)
    {
        register int page = (address >> 24) & 15;
        register write_func func = m_write_table[page];

        m_cycles -= m_mem_cycles32[page];

        (this->*func)(address, value & 0xFF);
        (this->*func)(address + 1, (value >> 8) & 0xFF);
        (this->*func)(address + 2, (value >> 16) & 0xFF);
        (this->*func)(address + 3, (value >> 24) & 0xFF);
    }

    u8 cpu::read_bios(u32 address)
    {
        if (address >= 0x4000) return 0;
        return m_bios[address];
    }

    u8 cpu::read_wram(u32 address)
    {
        return m_wram[address % 0x40000];
    }


    u8 cpu::read_iram(u32 address)
    {
        return m_iram[address % 0x8000];
    }

    u8 cpu::read_pal(u32 address)
    {
        return m_pal[address % 0x400];
    }

    u8 cpu::read_vram(u32 address)
    {
        address %= 0x20000;

        if (address >= 0x18000)
            address &= ~0x8000;

        return m_vram[address];
    }

    u8 cpu::read_oam(u32 address)
    {
        return m_oam[address % 0x400];
    }

    u8 cpu::read_rom(u32 address)
    {
        address &= 0x1FFFFFF;

        if (address < m_rom_size)
        {
            return m_rom[address];
        }

        return 0;
    }

    u8 cpu::read_save(u32 address)
    {
        if (m_backup)
        {
            return m_backup->read_byte(address);
        }
    }

    u8 cpu::read_invalid(u32 address)
    {
        logger::log<LOG_ERROR>("illegal read from 0x{0:x}", address);
        return 0;
    }

    void cpu::write_wram(u32 address, u8 value)
    {
        m_wram[address % 0x40000] = value;
    }

    void cpu::write_iram(u32 address, u8 value)
    {
        m_iram[address % 0x8000] = value;
    }

    void cpu::write_pal(u32 address, u8 value)
    {
        m_pal[address % 0x400] = value;
    }

    void cpu::write_vram(u32 address, u8 value)
    {
        address %= 0x20000;

        if (address >= 0x18000)
            address &= ~0x8000;

        m_vram[address] = value;
    }

    void cpu::write_oam(u32 address, u8 value)
    {
        m_oam[address % 0x400] = value;
    }

    void cpu::write_save(u32 address, u8 value)
    {
        if (m_backup)
        {
            m_backup->write_byte(address, value);
        }
    }

    void cpu::write_invalid(u32 address, u8 value)
    {
        logger::log<LOG_ERROR>("illegal write to 0x{0:x} = 0x{1:x}", address, value);
    }
}
