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

    auto& ppu_io = ppu.mmio;

    switch (address) {
    case DISPCNT+0: return ppu_io.dispcnt.Read(0);
    case DISPCNT+1: return ppu_io.dispcnt.Read(1);
    }
    return 0;
}

void CPU::WriteMMIO(std::uint32_t address, std::uint8_t value) {
    std::printf("[W][MMIO] 0x%08x=0x%02x\n", address, value);

    auto& ppu_io = ppu.mmio;

    switch (address) {
    case DISPCNT+0: ppu_io.dispcnt.Write(0, value); break;
    case DISPCNT+1: ppu_io.dispcnt.Write(1, value); break;
    }
}

} // namespace GBA
} // namespace NanoboyAdvance
