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

u8 bus_read_byte(u32 address, int flags) final {
    register int page = (address >> 24) & 15;
    register read_func func = s_read_table[page];

    m_cycles -= s_mem_cycles8_16[page];

    return (this->*func)(address);
}

u16 bus_read_hword(u32 address, int flags) final {
    register int page = (address >> 24) & 15;
    register read_func func = s_read_table[page];

    m_cycles -= s_mem_cycles8_16[page];

    return (this->*func)(address) | ((this->*func)(address + 1) << 8);
}

u32 bus_read_word(u32 address, int flags) final {
    switch ((address >> 24) & 15) {
        // BIOS memory
        case 0x0: {
            if (address >= 0x4000) {
                return 0;
            }
            return *(u32*)(&m_bios[address]);
        }
        // WRAM memory
        case 0x2: {
            return  bus_read_hword(address,     flags) |
                   (bus_read_hword(address + 2, flags) << 16);
        }
        // IWRAM memory
        case 0x3: {
            return *(u32*)(&m_iram[address & 0x7FFF]);
        }
        // IO
        case 0x4: {
            return  read_mmio(address) |
                   (read_mmio(address + 1) << 8 ) |
                   (read_mmio(address + 2) << 16) |
                   (read_mmio(address + 3) << 24);
        }
        // PRAM memory
        case 0x5: {
            return  bus_read_hword(address,     flags) |
                   (bus_read_hword(address + 2, flags) << 16);
        }
        // VRAM memory
        case 0x6: {
            return  bus_read_hword(address,     flags) |
                   (bus_read_hword(address + 2, flags) << 16);
        }
        // OAM
        case 0x7: {
            return *(u32*)(&m_oam[address & 0x3FF]);
        }
        // ROM (cartridge)
        case 0x8:
        case 0x9: {
            address &= 0x1FFFFFF;
            if (address >= m_rom_size) {
                return 0;
            }
            return  *(u32*)(&m_rom[address]);
        }
        // SRAM/FLASH
        case 0xE: {
            if (!m_backup) {
                return 0;
            }
            return m_backup->read_byte(address) * 0x01010101;
        }
            
        default: {
            register int page = (address >> 24) & 15;
            register read_func func = s_read_table[page];

            m_cycles -= s_mem_cycles32[page];

            return (this->*func)(address) |
                   ((this->*func)(address + 1) << 8) |
                   ((this->*func)(address + 2) << 16) |
                   ((this->*func)(address + 3) << 24);
        }
    }
}

void bus_write_byte(u32 address, u8 value, int flags) final {
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

void bus_write_hword(u32 address, u16 value, int flags) final {
    register int page = (address >> 24) & 15;
    register write_func func = s_write_table[page];

    m_cycles -= s_mem_cycles8_16[page];

    (this->*func)(address + 0, value & 0xFF);
    (this->*func)(address + 1, value >> 8);
}

void bus_write_word(u32 address, u32 value, int flags) final {
    register int page = (address >> 24) & 15;
    register write_func func = s_write_table[page];

    m_cycles -= s_mem_cycles32[page];

    (this->*func)(address + 0, (value >> 0) & 0xFF);
    (this->*func)(address + 1, (value >> 8) & 0xFF);
    (this->*func)(address + 2, (value >> 16) & 0xFF);
    (this->*func)(address + 3, (value >> 24) & 0xFF);
}

#endif
