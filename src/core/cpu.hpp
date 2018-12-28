/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "arm/arm.hpp"
#include "arm/interface.hpp"

namespace NanoboyAdvance {
namespace GBA {

class CPU : ARM::Interface {

public:
    void Init() {
        cpu.Reset();
        cpu.SetInterface(this);
    }

    /* Memory bus implementation (ReadByte, ...) */
    #include "bus.inl"

    void Tick(int cycles) final { }

private:
    ARM::ARM7 cpu;

    struct SystemMemory {
        std::uint8_t bios    [0x04000];
        std::uint8_t wram    [0x40000];
        std::uint8_t iram    [0x08000];
        std::uint8_t palette [0x00400];
        std::uint8_t oam     [0x00400];
        std::uint8_t vram    [0x18000];

        struct ROM {
            std::uint8_t* data;
            Save* save;
            size_t size;
        } rom;

        /* Last opcode fetched from BIOS memory. */
        u32 bios_opcode;

        /* TODO: remove this hack. */
        std::uint8_t mmio[0x800];
    } memory;
};

} // namespace GBA
} // namespace NanoboyAdvance
