/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "cpu.hpp"
#include "mmio.hpp"
#include <cstdio>

namespace NanoboyAdvance {
namespace GBA {

auto CPU::ReadMMIO(std::uint32_t address) -> std::uint8_t {
    std::printf("[R][MMIO] 0x%08x\n", address);
    return 0;
}

void CPU::WriteMMIO(std::uint32_t address, std::uint8_t value) {
    std::printf("[W][MMIO] 0x%08x=0x%02x\n", address, value);
}

} // namespace GBA
} // namespace NanoboyAdvance
