/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* TODO: Add support for Big-Endian architectures. */

#define READ_FAST_8(buffer, address)  *(uint8_t*) (&buffer[address])
#define READ_FAST_16(buffer, address) *(uint16_t*)(&buffer[address])
#define READ_FAST_32(buffer, address) *(uint32_t*)(&buffer[address])

#define WRITE_FAST_8(buffer, address, value)  *(uint8_t*) (&buffer[address]) = value;
#define WRITE_FAST_16(buffer, address, value) *(uint16_t*)(&buffer[address]) = value;
#define WRITE_FAST_32(buffer, address, value) *(uint32_t*)(&buffer[address]) = value;

std::uint32_t ReadBIOS(std::uint32_t address) {
    if (cpu.GetState().r15 >= 0x4000) {
        return memory.bios_opcode;
    }
    if (address >= 0x4000) {
        /* TODO: check if this matches HW behaviour. */
        memory.bios_opcode = 0;
        return 0;
    }

    return (memory.bios_opcode = READ_FAST_32(memory.bios, address));
}

std::uint8_t ReadByte(std::uint32_t address, ARM::AccessType type) final {
    int page = (address >> 24) & 15;

    switch (page) {
        case 0x0: return ReadBIOS(address);
        case 0x2: return READ_FAST_8(memory.wram, address & 0x3FFFF);
        case 0x3: return READ_FAST_8(memory.iram, address & 0x7FFF);
        case 0x4: {
            //std::printf("[R][MMIO] 0x%08x\n", address);
            static int dank = 0;

            std::uint32_t reg = address & 0x3FF;

            /* Fool armwrestler into thinking we emulate the VBlank flag. */
            if (reg == 4) {
                dank ^= 1;
                return dank;
            }
            if (reg == 0x130 || reg == 0x131)
                return 0xFF;

            return 0;
            //return  readMMIO(address) |
            //       (readMMIO(address + 1) << 8 );
        }
        case 0x5: return READ_FAST_8(memory.palette, address & 0x3FF);
        case 0x6: {
            address &= 0x1FFFF;
            if (address >= 0x18000)
                address &= ~0x8000;
            return READ_FAST_8(memory.vram, address);
        }
        case 0x7: return READ_FAST_8(memory.oam, address & 0x3FF);
        case 0x8: case 0x9:
        case 0xA: case 0xB:
        case 0xC: case 0xD: {
            address &= 0x1FFFFFF;
            // if (IS_GPIO_ACCESS(address) && gpio->isReadable()) {
            //     return gpio->read(address);
            // }
            if (address >= memory.rom.size)
                return address / 2;
            return READ_FAST_8(memory.rom.data, address);
        }
        // case 0xE: {
        //     if (!memory.rom.save || cart->type == SAVE_EEPROM) {
        //         return 0;
        //     }
        //     return memory.rom.save->read8(address);
        // }
        default: return 0;
    }
}

std::uint16_t ReadHalf(std::uint32_t address, ARM::AccessType type) final {
    int page = (address >> 24) & 15;

    switch (page) {
        case 0x0: {
            return ReadBIOS(address);
        }
        case 0x2: return READ_FAST_16(memory.wram, address & 0x3FFFF);
        case 0x3: return READ_FAST_16(memory.iram, address & 0x7FFF );
        case 0x4: {
            return  ReadByte(address, type) |
                   (ReadByte(address + 1, type) << 8);
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
        // case 0xD: {
        //     // Must check if this is an EEPROM access or ordinary ROM mirror read.
        //     if (IS_EEPROM_ACCESS(address)) {
        //         if (~flags & M_DMA) {
        //             return 1;
        //         }
        //         return memory.rom.save->read8(address);
        //     }
        // }
        case 0x8: case 0x9:
        case 0xA: case 0xB:
        case 0xC: {
            address &= 0x1FFFFFF;
            // if (IS_GPIO_ACCESS(address) && gpio->isReadable()) {
            //     return  gpio->read(address) |
            //            (gpio->read(address + 1) << 8);
            // }
            if (address >= memory.rom.size) {
                return address >> 1;
            }
            return READ_FAST_16(memory.rom.data, address);
        }
        // case 0xE: {
        //     if (!memory.rom.save || cart->type == SAVE_EEPROM) {
        //         return 0;
        //     }
        //     return memory.rom.save->read8(address) * 0x0101;
        // }

        default: return 0;
    }
}

std::uint32_t ReadWord(std::uint32_t address, ARM::AccessType type) final {
    int page = (address >> 24) & 15;

    switch (page) {
        case 0x0: {
            return ReadBIOS(address);
        }
        case 0x2: return READ_FAST_32(memory.wram, address & 0x3FFFF);
        case 0x3: return READ_FAST_32(memory.iram, address & 0x7FFF );
        case 0x4: {
            return  ReadByte(address, type) |
                   (ReadByte(address + 1, type) << 8 ) |
                   (ReadByte(address + 2, type) << 16) |
                   (ReadByte(address + 3, type) << 24);
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
            // if (IS_GPIO_ACCESS(address) && gpio->isReadable()) {
            //     return  gpio->read(address)            |
            //            (gpio->read(address + 1) << 8)  |
            //            (gpio->read(address + 2) << 16) |
            //            (gpio->read(address + 3) << 24);
            // }
            if (address >= memory.rom.size) {
                return ( (address      >> 1) &  0xFFFF) |
                       (((address + 2) >> 1) << 16    );
            }
            return READ_FAST_32(memory.rom.data, address);
        }
        // case 0xE: {
        //     if (!memory.rom.save  || cart->type == SAVE_EEPROM) {
        //         return 0;
        //     }
        //     return memory.rom.save->read8(address) * 0x01010101;
        // }

        default: return 0;
    }
}

void WriteByte(std::uint32_t address, std::uint8_t value, ARM::AccessType type) final {
    int page = (address >> 24) & 15;

    switch (page) {
        case 0x2: WRITE_FAST_8(memory.wram, address & 0x3FFFF, value); break;
        case 0x3: WRITE_FAST_8(memory.iram, address & 0x7FFF,  value); break;
        case 0x4: {
            std::printf("[W][MMIO] 0x%08x = 0x%02x\n", address, value);
            //writeMMIO(address, value & 0xFF);
            break;
        }
        case 0x5: WRITE_FAST_16(memory.palette, address & 0x3FF, value * 0x0101); break;
        case 0x6: {
            //std::printf("[W][VRAM] 0x%08x = 0x%02x\n", address, value);
            address &= 0x1FFFF;
            if (address >= 0x18000)
                address &= ~0x8000;
            WRITE_FAST_16(memory.vram, address, value * 0x0101);
            break;
        }
        case 0x7: WRITE_FAST_16(memory.oam, address & 0x3FF, value * 0x0101); break;
        case 0xE: {
            //if (!memory.rom.save || cart->type == SAVE_EEPROM)
            //    break;
            //memory.rom.save->write8(address, value);
            break;
        }
        default: break; /* TODO: log illegal writes. */
    }
}

void WriteHalf(std::uint32_t address, std::uint16_t value, ARM::AccessType type) final {
    int page = (address >> 24) & 15;

    switch (page) {
        case 0x2: WRITE_FAST_16(memory.wram, address & 0x3FFFF, value); break;
        case 0x3: WRITE_FAST_16(memory.iram, address & 0x7FFF,  value); break;
        case 0x4: {
            WriteByte(address, value & 0xFF, type);
            WriteByte(address + 1, (value >> 8)  & 0xFF, type);
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

        case 0x8: case 0x9:
        case 0xA: case 0xB:
        // case 0xC: {
        //     address &= 0x1FFFFFF;
        //     if (IS_GPIO_ACCESS(address)) {
        //         gpio->write(address+0, value&0xFF);
        //         gpio->write(address+1, value>>8);
        //     }
        //     break;
        // }

        // EEPROM write
        // case 0xD: {
        //      if (IS_EEPROM_ACCESS(address)) {
        //         if (~flags & M_DMA) {
        //             break;
        //         }
        //         memory.rom.save->write8(address, value);
        //         break;
        //     }
        //     address &= 0x1FFFFFF;
        //     if (IS_GPIO_ACCESS(address)) {
        //         gpio->write(address+0, value&0xFF);
        //         gpio->write(address+1, value>>8);
        //         break;
        //     }
        //     break;
        // }
        //
        // case 0xE: {
        //     if (!memory.rom.save || cart->type == SAVE_EEPROM) {
        //         break;
        //     }
        //     memory.rom.save->write8(address + 0, value);
        //     memory.rom.save->write8(address + 1, value);
        //     break;
        // }
        default: break; // TODO: throw error
    }
}

void WriteWord(std::uint32_t address, std::uint32_t value, ARM::AccessType type) final {
    int page = (address >> 24) & 15;

    switch (page) {
        case 0x2: WRITE_FAST_32(memory.wram, address & 0x3FFFF, value); break;
        case 0x3: WRITE_FAST_32(memory.iram, address & 0x7FFF,  value); break;
        case 0x4: {
            WriteByte(address, value & 0xFF, type);
            WriteByte(address + 1, (value >> 8)  & 0xFF, type);
            WriteByte(address + 2, (value >> 16) & 0xFF, type);
            WriteByte(address + 3, (value >> 24) & 0xFF, type);
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

        case 0x8: case 0x9:
        case 0xA: case 0xB:
        // case 0xC: case 0xD: {
        //     // TODO: check if 32-bit EEPROM accesses are possible.
        //     address &= 0x1FFFFFF;
        //     if (IS_GPIO_ACCESS(address)) {
        //         gpio->write(address+0, (value>>0) &0xFF);
        //         gpio->write(address+1, (value>>8) &0xFF);
        //         gpio->write(address+2, (value>>16)&0xFF);
        //         gpio->write(address+3, (value>>24)&0xFF);
        //     }
        //     break;
        // }

        // case 0xE: {
        //     if (!memory.rom.save || cart->type == SAVE_EEPROM) {
        //         break;
        //     }
        //     memory.rom.save->write8(address + 0, value);
        //     memory.rom.save->write8(address + 1, value);
        //     memory.rom.save->write8(address + 2, value);
        //     memory.rom.save->write8(address + 3, value);
        //     break;
        // }
        default: break; // TODO: throw error
    }
}

#undef READ_FAST_8
#undef READ_FAST_16
#undef READ_FAST_32
#undef WRITE_FAST_8
#undef WRITE_FAST_16
#undef WRITE_FAST_32
