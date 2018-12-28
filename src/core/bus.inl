/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

std::uint8_t ReadByte(std::uint32_t address, ARM::AccessType type) final {
    return 0;
}

std::uint16_t ReadHalf(std::uint32_t address, ARM::AccessType type) final {
    return 0;
}

std::uint32_t ReadWord(std::uint32_t address, ARM::AccessType type) final {
    return 0;
}

void WriteByte(std::uint32_t address, std::uint8_t  value, ARM::AccessType type) final {
}

void WriteHalf(std::uint32_t address, std::uint16_t value, ARM::AccessType type) final {
}

void WriteWord(std::uint32_t address, std::uint32_t value, ARM::AccessType type) final {
}
