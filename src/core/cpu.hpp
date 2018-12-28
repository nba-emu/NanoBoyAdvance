/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "arm/arm.hpp"
#include "arm/interface.hpp"
#include <cstring>

namespace NanoboyAdvance {
namespace GBA {

class CPU : ARM::Interface {

public:
    void Reset() {
        cpu.Reset();
        cpu.SetInterface(this);

        /* Clear-out all memory buffers. */
        std::memset(memory.wram,    0, 0x40000);
        std::memset(memory.iram,    0, 0x08000);
        std::memset(memory.palette, 0, 0x00400);
        std::memset(memory.oam,     0, 0x00400);
        std::memset(memory.vram,    0, 0x18000);
        std::memset(memory.mmio,    0, 0x00800);

        cpu.GetState().r15 = 0x08000000;
    }

    /* Memory bus implementation (ReadByte, ...) */
    #include "bus.inl"

    void Tick(int cycles) final { }
    void SWI(std::uint32_t call_id) final { }

    /* TODO: Evaluate if there are better ways? */
    void SetSlot1(uint8_t* rom, size_t size) {
        memory.rom.data = rom;
        memory.rom.size = size;
    }

    void Dork() {
        auto& state = cpu.GetState();
        cpu.Run();
        // for (int i = 0; i < 16; i++) {
        //     std::printf("r%d: 0x%08x\n", i, state.reg[i]);
        // }
    }
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
            //Save* save;
            size_t size;
        } rom;

        /* Last opcode fetched from BIOS memory. */
        std::uint32_t bios_opcode;

        /* TODO: remove this hack. */
        std::uint8_t mmio[0x800];
    } memory;
};

} // namespace GBA
} // namespace NanoboyAdvance
