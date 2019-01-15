/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstring>
#include "cpu.hpp"

using namespace NanoboyAdvance::GBA;

constexpr int CPU::s_ws_nseq[4]; /* Non-sequential SRAM/WS0/WS1/WS2 */
constexpr int CPU::s_ws_seq0[2]; /* Sequential WS0 */
constexpr int CPU::s_ws_seq1[2]; /* Sequential WS1 */
constexpr int CPU::s_ws_seq2[2]; /* Sequential WS2 */

CPU::CPU(Config* config)
    : config(config)
    , ppu(this)
{
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

    /* Reset waitstates. */
    mmio.waitcnt.sram = 0;
    mmio.waitcnt.ws0_n = 0;
    mmio.waitcnt.ws0_s = 0;
    mmio.waitcnt.ws1_n = 0;
    mmio.waitcnt.ws1_s = 0;
    mmio.waitcnt.ws2_n = 0;
    mmio.waitcnt.ws2_s = 0;
    mmio.waitcnt.phi = 0;
    mmio.waitcnt.prefetch = 0;
    mmio.waitcnt.cgb = 0;
    /* TODO: implement register 0x04000800. */
    for (int i = 0; i < 2; i++) {
        cycles16[i][0x0] = 1;
        cycles32[i][0x0] = 1;
        cycles16[i][0x1] = 1;
        cycles32[i][0x1] = 1;
        cycles16[i][0x2] = 3;
        cycles32[i][0x2] = 6;
        cycles16[i][0x3] = 1;
        cycles32[i][0x3] = 1;
        cycles16[i][0x4] = 1;
        cycles32[i][0x4] = 1;
        cycles16[i][0x5] = 1;
        cycles32[i][0x5] = 2;
        cycles16[i][0x6] = 1;
        cycles32[i][0x6] = 2;
        cycles16[i][0x7] = 1;
        cycles32[i][0x7] = 1;
        cycles16[i][0xF] = 1;
        cycles32[i][0xF] = 1;
    }
    UpdateCycleLUT();

    mmio.haltcnt = HaltControl::RUN;

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
        int go_for = ppu.wait_cycles;

        run_until += go_for;

        while (run_until > 0) {
            std::uint16_t fire = mmio.irq_ie & mmio.irq_if;

            if (mmio.haltcnt == HaltControl::HALT && fire)
                mmio.haltcnt = HaltControl::RUN;

            if (mmio.haltcnt == HaltControl::RUN) {
                if (mmio.irq_ime && fire)
                    cpu.SignalIrq();
                cpu.Run();
            } else {
                run_until = 0;
            }
        }

        ppu.wait_cycles -= go_for + run_until;
        if (ppu.wait_cycles <= 0) {
            ppu.Tick();
        }
        cycles -= go_for;
    }
}

void CPU::UpdateCycleLUT() {
    const auto& waitcnt = mmio.waitcnt;

    /* SRAM timing. */
    cycles16[0][0xE] = s_ws_nseq[waitcnt.sram];
    cycles16[1][0xE] = s_ws_nseq[waitcnt.sram];
    cycles32[0][0xE] = s_ws_nseq[waitcnt.sram];
    cycles32[1][0xE] = s_ws_nseq[waitcnt.sram];

    /* ROM: WS0/WS1/WS2 non-sequential timing. */
    cycles16[0][0x8] = cycles16[0][0x9] = 1 + s_ws_nseq[waitcnt.ws0_n];
    cycles16[0][0xA] = cycles16[0][0xB] = 1 + s_ws_nseq[waitcnt.ws1_n];
    cycles16[0][0xC] = cycles16[0][0xD] = 1 + s_ws_nseq[waitcnt.ws2_n];

    /* ROM: WS0/WS1/WS2 sequential timing. */
    cycles16[1][0x8] = cycles16[1][0x9] = 1 + s_ws_seq0[waitcnt.ws0_s];
    cycles16[1][0xA] = cycles16[1][0xB] = 1 + s_ws_seq1[waitcnt.ws1_s];
    cycles16[1][0xC] = cycles16[1][0xD] = 1 + s_ws_seq2[waitcnt.ws2_s];

    /* ROM: WS0/WS1/WS2 32-bit non-sequential access: 1N access, 1S access */
    cycles32[0][0x8] = cycles32[0][0x9] = cycles16[0][0x8] + cycles16[1][0x8];
    cycles32[0][0xA] = cycles32[0][0xB] = cycles16[0][0xA] + cycles16[1][0x8];
    cycles32[0][0xC] = cycles32[0][0xD] = cycles16[0][0xC] + cycles16[1][0x8];

    /* ROM: WS0/WS1/WS2 32-bit sequential access: 2S accesses */
    cycles32[1][0x8] = cycles32[1][0x9] = cycles16[1][0x8] * 2;
    cycles32[1][0xA] = cycles32[1][0xB] = cycles16[1][0xA] * 2;
    cycles32[1][0xC] = cycles32[1][0xD] = cycles16[1][0xC] * 2;
}
