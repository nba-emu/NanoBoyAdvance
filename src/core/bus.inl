/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

std::uint8_t ReadByte(std::uint32_t address, ARM::AccessType type) final {
    int page = (address >> 24) & 15;
    if (page == 8) {
        return memory.rom.data[address & 0xFFFFFF];
    } else {
        std::printf("[R] 0x%08x\n", address);
        return 0;
    }
    return 0;
}

std::uint16_t ReadHalf(std::uint32_t address, ARM::AccessType type) final {
    return ReadByte(address + 0, type) |
          (ReadByte(address + 1, type) << 8);
}

std::uint32_t ReadWord(std::uint32_t address, ARM::AccessType type) final {
    return ReadHalf(address + 0, type) |
          (ReadHalf(address + 2, type) << 16);
}

void WriteByte(std::uint32_t address, std::uint8_t value, ARM::AccessType type) final {
    /* TODO */
    std::printf("[W] 0x%08x = 0x%02x\n", address, value);
}

void WriteHalf(std::uint32_t address, std::uint16_t value, ARM::AccessType type) final {
    WriteByte(address + 0, (uint8_t)value, type);
    WriteByte(address + 1, value >> 8, type);
}

void WriteWord(std::uint32_t address, std::uint32_t value, ARM::AccessType type) final {
    WriteHalf(address + 0, (uint16_t)value, type);
    WriteHalf(address + 2, value >> 16, type);
}
