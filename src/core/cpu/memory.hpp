/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  * 
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#pragma once 

#ifdef CPU_INCLUDE

u8 bus_read_byte(u32 address) final {
    register int page = (address >> 24) & 15;
    register read_func func = s_read_table[page];

    m_cycles -= s_mem_cycles8_16[page];

    return (this->*func)(address);
}

u16 bus_read_hword(u32 address) final {
    register int page = (address >> 24) & 15;
    register read_func func = s_read_table[page];

    m_cycles -= s_mem_cycles8_16[page];

    return (this->*func)(address) | ((this->*func)(address + 1) << 8);
}

u32 bus_read_word(u32 address) final {
    register int page = (address >> 24) & 15;
    register read_func func = s_read_table[page];

    m_cycles -= s_mem_cycles32[page];

    return (this->*func)(address) |
           ((this->*func)(address + 1) << 8) |
           ((this->*func)(address + 2) << 16) |
           ((this->*func)(address + 3) << 24);
}

void bus_write_byte(u32 address, u8 value) final {
    register int page = (address >> 24) & 15;
    register write_func func = s_write_table[page];

    m_cycles -= s_mem_cycles8_16[page];

    // handle 16-bit data busses. might reduce condition?
    if (page == 5 || page == 6 || page == 7) {
        (this->*func)(address & ~1, value);
        (this->*func)((address & ~1) + 1, value);
        return;
    }

    (this->*func)(address, value);
}

void bus_write_hword(u32 address, u16 value) final {
    register int page = (address >> 24) & 15;
    register write_func func = s_write_table[page];

    m_cycles -= s_mem_cycles8_16[page];

    (this->*func)(address + 0, value & 0xFF);
    (this->*func)(address + 1, value >> 8);
}

void bus_write_word(u32 address, u32 value) final {
    register int page = (address >> 24) & 15;
    register write_func func = s_write_table[page];

    m_cycles -= s_mem_cycles32[page];

    (this->*func)(address + 0, (value >> 0) & 0xFF);
    (this->*func)(address + 1, (value >> 8) & 0xFF);
    (this->*func)(address + 2, (value >> 16) & 0xFF);
    (this->*func)(address + 3, (value >> 24) & 0xFF);
}

#endif
