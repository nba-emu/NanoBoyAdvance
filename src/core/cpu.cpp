/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstring>
#include "cpu.hpp"

namespace NanoboyAdvance {
namespace GBA {

CPU::CPU(Config* config) : config(config),
                           ppu(config, this) {
    Reset();
}
    
void CPU::Reset() {
    cpu.Reset();
    cpu.SetInterface(this);

    auto& state = cpu.GetState();

    state.bank[ARM::BANK_SVC][ARM::BANK_R13] = 0x03007FE0; 
    state.bank[ARM::BANK_IRQ][ARM::BANK_R13] = 0x03007FA0;
    state.reg[13] = 0x03007F00;
    state.cpsr.f.mode = ARM::MODE_USR;
    state.r15 = 0x08000000;

    /* Clear-out all memory buffers. */
    std::memset(memory.bios, 0, 0x04000);
    std::memset(memory.wram, 0, 0x40000);
    std::memset(memory.iram, 0, 0x08000);
    std::memset(memory.pram, 0, 0x00400);
    std::memset(memory.oam,  0, 0x00400);
    std::memset(memory.vram, 0, 0x18000);

    /* Load BIOS. This really should not be done here. */
    size_t size;
    auto file = std::fopen("bios.bin", "rb");
    std::uint8_t* rom;

    if (file == nullptr) {
        std::puts("Error: unable to open bios.bin");
        while (1) {}
        return;
    }

    std::fseek(file, 0, SEEK_END);
    size = std::ftell(file);
    std::fseek(file, 0, SEEK_SET);

    if (size > 0x4000) {
        std::puts("Error: BIOS image too large.");
        return;
    }

    if (std::fread(memory.bios, 1, size, file) != size) {
        std::puts("Error: unable to fully read the ROM.");
        return;
    }

    /* Reset interrupt control. */
    mmio.irq_ie = 0;
    mmio.irq_if = 0;
    mmio.irq_ime = 0;

    mmio.haltcnt = SYSTEM_RUN;

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
            std::uint16_t fire = mmio.irq_ie & mmio.irq_if;

            if (mmio.haltcnt == SYSTEM_HALT && fire)
                mmio.haltcnt = SYSTEM_RUN;

            if (mmio.haltcnt == SYSTEM_RUN) {
                if (mmio.irq_ime && fire)
                    cpu.SignalIrq();
                cpu.Run();
            }
        }

        ppu.Tick();
        cycles -= run_until;
    }
}

} // namespace NanoboyAdvance
} // namespace GBA
