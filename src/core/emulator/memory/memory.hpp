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

#define IS_EEPROM_ACCESS(address) memory.rom.save && cart->type == SAVE_EEPROM &&\
                                  ((~memory.rom.size & 0x02000000) || address >= 0x0DFFFF00)

auto readBIOS(u32 address) -> u32 {
    if (address >= 0x4000) {
        return 0;
    }
    if (context().r15 >= 0x4000) {
        return memory.bios_opcode;
    }
    return memory.bios_opcode = READ_FAST_32(memory.bios, address);
}

// CAREFUL: "flags & M_SEQ" only works because M_SEQ currently equals to "1".

u8 busRead8(u32 address, int flags) final {
    int page = (address >> 24) & 15;

    // poor mans cycle counting
    cycles_left -= cycles[flags & M_SEQ][page];

    switch (page) {
        case 0x0: {
            return readBIOS(address);
        }
        case 0x2: return READ_FAST_8(memory.wram, address & 0x3FFFF);
        case 0x3: return READ_FAST_8(memory.iram, address & 0x7FFF );
        case 0x4: {
            return  readMMIO(address) |
                   (readMMIO(address + 1) << 8 );
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
            if (address >= memory.rom.size) {
                return address >> 1;
            }
            return READ_FAST_8(memory.rom.data, address);
        }
        case 0xE: {
            if (!memory.rom.save || cart->type == SAVE_EEPROM) {
                return 0;
            }
            return memory.rom.save->read8(address);
        }
        default: return 0;
    }
}

u16 busRead16(u32 address, int flags) final {
    int page = (address >> 24) & 15;

    // poor mans cycle counting
    cycles_left -= cycles[flags & M_SEQ][page];

    switch (page) {
        case 0x0: {
            return readBIOS(address);
        }
        case 0x2: return READ_FAST_16(memory.wram, address & 0x3FFFF);
        case 0x3: return READ_FAST_16(memory.iram, address & 0x7FFF );
        case 0x4: {
            return  readMMIO(address) |
                   (readMMIO(address + 1) << 8 );
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

        // 0x0DXXXXXX may be used to read/write from EEPROM
        case 0xD: {
            // Must check if this is an EEPROM access or ordinary ROM mirror read.
            if (IS_EEPROM_ACCESS(address)) {
                if (~flags & M_DMA) {
                    return 1;
                }
                return memory.rom.save->read8(address);
            }
        }
        case 0x8: case 0x9:
        case 0xA: case 0xB:
        case 0xC: {
            address &= 0x1FFFFFF;
            if (address >= memory.rom.size) {
                return address >> 1;
            }
            return READ_FAST_16(memory.rom.data, address);
        }
        case 0xE: {
            if (!memory.rom.save || cart->type == SAVE_EEPROM) {
                return 0;
            }
            return memory.rom.save->read8(address) * 0x0101;
        }

        default: return 0;
    }
}

u32 busRead32(u32 address, int flags) final {
    const int page = (address >> 24) & 15;

    // poor mans cycle counting
    cycles_left -= cycles32[(flags & M_SEQ)?1:0][page];

    switch (page) {
        case 0x0: {
            return readBIOS(address);
        }
        case 0x2: return READ_FAST_32(memory.wram, address & 0x3FFFF);
        case 0x3: return READ_FAST_32(memory.iram, address & 0x7FFF );
        case 0x4: {
            return  readMMIO(address) |
                   (readMMIO(address + 1) << 8 ) |
                   (readMMIO(address + 2) << 16) |
                   (readMMIO(address + 3) << 24);
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
            if (address >= memory.rom.size) {
                return ( (address      >> 1) &  0xFFFF) |
                       (((address + 2) >> 1) << 16    );
            }
            return READ_FAST_32(memory.rom.data, address);
        }
        case 0xE: {
            if (!memory.rom.save  || cart->type == SAVE_EEPROM) {
                return 0;
            }
            return memory.rom.save->read8(address) * 0x01010101;
        }

        default: return 0;
    }
}

void busWrite8(u32 address, u8 value, int flags) final {
    int page = (address >> 24) & 15;

    // poor mans cycle counting
    cycles_left -= cycles[flags & M_SEQ][page];

    switch (page) {
        case 0x2: WRITE_FAST_8(memory.wram, address & 0x3FFFF, value); break;
        case 0x3: WRITE_FAST_8(memory.iram, address & 0x7FFF,  value); break;
        case 0x4: {
            writeMMIO(address, value & 0xFF);
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
            if (!memory.rom.save || cart->type == SAVE_EEPROM) {
                break;
            }
            memory.rom.save->write8(address, value);
            break;
        }
        default: break; // TODO: throw error
    }
}

void busWrite16(u32 address, u16 value, int flags) final {
    int page = (address >> 24) & 15;

    // poor mans cycle counting
    cycles_left -= cycles[flags & M_SEQ][page];

    switch (page) {
        case 0x2: WRITE_FAST_16(memory.wram, address & 0x3FFFF, value); break;
        case 0x3: WRITE_FAST_16(memory.iram, address & 0x7FFF,  value); break;
        case 0x4: {
            writeMMIO(address, value & 0xFF);
            writeMMIO(address + 1, (value >> 8)  & 0xFF);
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

        // EEPROM write
        case 0xD: {
            if (IS_EEPROM_ACCESS(address)) {
                if (~flags & M_DMA) {
                    break;
                }
                memory.rom.save->write8(address, value);
            }
            break;
        }

        case 0xE: {
            if (!memory.rom.save || cart->type == SAVE_EEPROM) {
                break;
            }
            memory.rom.save->write8(address + 0, value);
            memory.rom.save->write8(address + 1, value);
            break;
        }
        default: break; // TODO: throw error
    }
}

void busWrite32(u32 address, u32 value, int flags) final {
    int page = (address >> 24) & 15;

    // poor mans cycle counting
    cycles_left -= cycles32[(flags & M_SEQ)?1:0][page];

    switch (page) {
        case 0x2: WRITE_FAST_32(memory.wram, address & 0x3FFFF, value); break;
        case 0x3: WRITE_FAST_32(memory.iram, address & 0x7FFF,  value); break;
        case 0x4: {
            writeMMIO(address, value & 0xFF);
            writeMMIO(address + 1, (value >> 8)  & 0xFF);
            writeMMIO(address + 2, (value >> 16) & 0xFF);
            writeMMIO(address + 3, (value >> 24) & 0xFF);
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
            if (!memory.rom.save || cart->type == SAVE_EEPROM) {
                break;
            }
            memory.rom.save->write8(address + 0, value);
            memory.rom.save->write8(address + 1, value);
            memory.rom.save->write8(address + 2, value);
            memory.rom.save->write8(address + 3, value);
            break;
        }
        default: break; // TODO: throw error
    }
}
