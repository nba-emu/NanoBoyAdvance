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

// TODO(accuracy): 
//     Verify that this kind of reading works for edge cases.
//     Theoretically all addresses that are being passed should be aligned
//     which hopefully should make it impossible to access two memory areas at the time.

// TODO(accuracy):
//     Handle FLASH/SRAM/EEPROM etc accordingly :/

//TODO: poor big-endian is crying right now :(
#define READ_FAST_8(buffer, address)  *(u8*) (&buffer[address])
#define READ_FAST_16(buffer, address) *(u16*)(&buffer[address])
#define READ_FAST_32(buffer, address) *(u32*)(&buffer[address])

#define WRITE_FAST_8(buffer, address, value)  *(u8*) (&buffer[address]) = value;
#define WRITE_FAST_16(buffer, address, value) *(u16*)(&buffer[address]) = value;
#define WRITE_FAST_32(buffer, address, value) *(u32*)(&buffer[address]) = value;

auto read_bios(u32 address) -> u32 {    
    if (address >= 0x4000) {
        return 0;
    }
    if (get_context().r15 >= 0x4000) {
        return bios_opcode;
    }
    return bios_opcode = READ_FAST_32(memory.bios, address);
}

u8 bus_read_byte(u32 address, int flags) final {
    int page = (address >> 24) & 15;

    // poor mans cycle counting
    m_cycles -= s_mem_cycles8_16[page];

    switch (page) {
        case 0x0: {
            return read_bios(address);
        }
        case 0x2: return READ_FAST_8(memory.wram, address & 0x3FFFF);
        case 0x3: return READ_FAST_8(memory.iram, address & 0x7FFF );
        case 0x4: {
            return  read_mmio(address) |
                   (read_mmio(address + 1) << 8 );
        }
        case 0x5: return READ_FAST_8(memory.palette, address & 0x3FF);
        case 0x6: {
            address &= 0x1FFFF;
            if (address >= 0x18000) {
                address &= ~0x8000;
            }
            return READ_FAST_8(memory.vram, address);
        }
        case 0x7: return READ_FAST_8(memory.oam, address & 0x3FF);
        case 0x8: case 0x9:
        case 0xA: case 0xB: 
        case 0xC: case 0xD: {
            address &= 0x1FFFFFF;
            if (address >= m_rom_size) {
                return address >> 1;
            }
            return READ_FAST_8(m_rom, address);
        }
        case 0xE: {
            if (!m_backup) {
                return 0;
            }
            return m_backup->read_byte(address);
        }
        default: return 0;
    }
}

u16 bus_read_hword(u32 address, int flags) final {
    int page = (address >> 24) & 15;
    
    // poor mans cycle counting
    m_cycles -= s_mem_cycles8_16[page];
    
    switch (page) {
        case 0x0: {
            return read_bios(address);
        }
        case 0x2: return READ_FAST_16(memory.wram, address & 0x3FFFF);
        case 0x3: return READ_FAST_16(memory.iram, address & 0x7FFF );
        case 0x4: {
            return  read_mmio(address) |
                   (read_mmio(address + 1) << 8 );
        }
        case 0x5: return READ_FAST_16(memory.palette, address & 0x3FF);
        case 0x6: {
            address &= 0x1FFFF;
            if (address >= 0x18000) {
                address &= ~0x8000;
            }
            return READ_FAST_16(memory.vram, address);
        }
        case 0x7: return READ_FAST_16(memory.oam, address & 0x3FF);
        case 0x8: case 0x9:
        case 0xA: case 0xB: 
        case 0xC: case 0xD: {
            address &= 0x1FFFFFF;
            if (address >= m_rom_size) {
                return address >> 1;
            }
            return READ_FAST_16(m_rom, address);
        }
        case 0xE: {
            if (!m_backup) {
                return 0;
            }
            return m_backup->read_byte(address) * 0x0101;
        }
        
        default: return 0;
    }
}

u32 bus_read_word(u32 address, int flags) final {
    const int page = (address >> 24) & 15;
    
    // poor mans cycle counting
    m_cycles -= s_mem_cycles32[page];
    
    switch (page) {
        case 0x0: {
            return read_bios(address);
        }
        case 0x2: return READ_FAST_32(memory.wram, address & 0x3FFFF);
        case 0x3: return READ_FAST_32(memory.iram, address & 0x7FFF );
        case 0x4: {
            return  read_mmio(address) |
                   (read_mmio(address + 1) << 8 ) |
                   (read_mmio(address + 2) << 16) |
                   (read_mmio(address + 3) << 24);
        }
        case 0x5: return READ_FAST_32(memory.palette, address & 0x3FF);
        case 0x6: {
            address &= 0x1FFFF;
            if (address >= 0x18000) {
                address &= ~0x8000;
            }
            return READ_FAST_32(memory.vram, address);
        }
        case 0x7: return READ_FAST_32(memory.oam, address & 0x3FF);
        case 0x8: case 0x9:
        case 0xA: case 0xB: 
        case 0xC: case 0xD: {
            address &= 0x1FFFFFF;
            if (address >= m_rom_size) {
                return ( (address      >> 1) &  0xFFFF) |
                       (((address + 2) >> 1) << 16    );
            }
            return READ_FAST_32(m_rom, address);
        }
        case 0xE: {
            if (!m_backup) {
                return 0;
            }
            return m_backup->read_byte(address) * 0x01010101;
        }
            
        default: return 0;
    }
}

void bus_write_byte(u32 address, u8 value, int flags) final {
    int page = (address >> 24) & 15;
    
    // poor mans cycle counting
    m_cycles -= s_mem_cycles8_16[page];
    
    switch (page) {
        case 0x2: WRITE_FAST_8(memory.wram, address & 0x3FFFF, value); break;
        case 0x3: WRITE_FAST_8(memory.iram, address & 0x7FFF,  value); break;
        case 0x4: {
            write_mmio(address, value & 0xFF);
            break;
        }
        case 0x5: WRITE_FAST_16(memory.palette, address & 0x3FF, value * 0x0101); break;
        case 0x6: {
            address &= 0x1FFFF;
            if (address >= 0x18000) {
                address &= ~0x8000;
            }
            WRITE_FAST_16(memory.vram, address, value * 0x0101);
            break;
        }
        case 0x7: WRITE_FAST_16(memory.oam, address & 0x3FF, value * 0x0101); break;
        case 0xE: {
            if (!m_backup) { 
                break;
            }
            m_backup->write_byte(address, value); 
            break;
        }
        default: break; // TODO: throw error
    }
}

void bus_write_hword(u32 address, u16 value, int flags) final {
    int page = (address >> 24) & 15;
    
    // poor mans cycle counting
    m_cycles -= s_mem_cycles8_16[page];
    
    switch (page) {
        case 0x2: WRITE_FAST_16(memory.wram, address & 0x3FFFF, value); break;
        case 0x3: WRITE_FAST_16(memory.iram, address & 0x7FFF,  value); break;
        case 0x4: {
            write_mmio(address, value & 0xFF);
            write_mmio(address + 1, (value >> 8)  & 0xFF);
            break;
        }
        case 0x5: WRITE_FAST_16(memory.palette, address & 0x3FF, value); break;
        case 0x6: {
            address &= 0x1FFFF;
            if (address >= 0x18000) {
                address &= ~0x8000;
            }
            WRITE_FAST_16(memory.vram, address, value);
            break;
        }
        case 0x7: WRITE_FAST_16(memory.oam, address & 0x3FF, value); break;
        case 0xE: {
            if (!m_backup) { 
                break;
            }
            m_backup->write_byte(address + 0, value); 
            m_backup->write_byte(address + 1, value); 
            break;
        }
        default: break; // TODO: throw error
    }
}

void bus_write_word(u32 address, u32 value, int flags) final {
    int page = (address >> 24) & 15;
    
    // poor mans cycle counting
    m_cycles -= s_mem_cycles32[page];
    
    switch (page) {
        case 0x2: WRITE_FAST_32(memory.wram, address & 0x3FFFF, value); break;
        case 0x3: WRITE_FAST_32(memory.iram, address & 0x7FFF,  value); break;
        case 0x4: {
            write_mmio(address, value & 0xFF);
            write_mmio(address + 1, (value >> 8)  & 0xFF);
            write_mmio(address + 2, (value >> 16) & 0xFF);
            write_mmio(address + 3, (value >> 24) & 0xFF);
            break;
        }
        case 0x5: WRITE_FAST_32(memory.palette, address & 0x3FF, value); break;
        case 0x6: {
            address &= 0x1FFFF;
            if (address >= 0x18000) {
                address &= ~0x8000;
            }
            WRITE_FAST_32(memory.vram, address, value);
            break;
        }
        case 0x7: WRITE_FAST_32(memory.oam, address & 0x3FF, value); break;
        case 0xE: {
            if (!m_backup) { 
                break;
            }
            m_backup->write_byte(address + 0, value); 
            m_backup->write_byte(address + 1, value); 
            m_backup->write_byte(address + 2, value); 
            m_backup->write_byte(address + 3, value); 
            break;
        }
        default: break; // TODO: throw error
    }
}

#endif
