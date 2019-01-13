/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "config.hpp"
#include "arm/arm.hpp"
#include "arm/interface.hpp"
#include "ppu/ppu.hpp"
#include <cstring>

namespace NanoboyAdvance {
namespace GBA {

class CPU : private ARM::Interface,
            private ARM::ARM7 {
    PPU ppu;

    Config* config;

    struct SystemMemory {
        std::uint8_t bios[0x04000];
        std::uint8_t wram[0x40000];
        std::uint8_t iram[0x08000];
        std::uint8_t pram[0x00400];
        std::uint8_t oam [0x00400];
        std::uint8_t vram[0x18000];

        struct ROM {
            std::uint8_t* data;
            size_t size;
        } rom;

        /* Last opcode fetched from BIOS memory. */
        std::uint32_t bios_opcode;
    } memory;

    /* Memory bus implementation (ReadByte, ...) */
    #include "bus.inl"

    void Tick(int cycles) final { }
    void SWI(std::uint32_t call_id) final { }

    auto ReadMMIO(std::uint32_t address) -> std::uint8_t;
    void WriteMMIO(std::uint32_t address, std::uint8_t value);

public:
    CPU(Config* config);

    void Reset();
    void SetSlot1(uint8_t* rom, size_t size);

    void RunFor(int cycles);
};

} // namespace GBA
} // namespace NanoboyAdvance
