/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "cpu.hpp"

namespace NanoboyAdvance {
namespace GBA {

CPU::CPU(Config* config) : config(config) {
    ppu.config = config;
    ppu.pram = memory.pram;
    ppu.vram = memory.vram;
    ppu.oam  = memory.oam;
    Reset();
}
    
void CPU::Reset() {
    cpu.Reset();
    cpu.SetInterface(this);
    cpu.GetState().r15 = 0x08000000;

    /* Clear-out all memory buffers. */
    std::memset(memory.wram, 0, 0x40000);
    std::memset(memory.iram, 0, 0x08000);
    std::memset(memory.pram, 0, 0x00400);
    std::memset(memory.oam,  0, 0x00400);
    std::memset(memory.vram, 0, 0x18000);

    ppu.Reset();
}

/* TODO: Does this really belong into the CPU class? */
void CPU::SetSlot1(uint8_t* rom, size_t size) {
    memory.rom.data = rom;
    memory.rom.size = size;
    Reset();
}

void CPU::RunFor(int cycles) {
    while (cycles > 0) {
        /* Get next event. */
        int run_until = ppu.wait_cycles;

        for (int i = 0; i < run_until / 4; i++) {
            cpu.Run();
        }

        ppu.Tick();
        cycles -= run_until;
    }
}

} // namespace NanoboyAdvance
} // namespace GBA
