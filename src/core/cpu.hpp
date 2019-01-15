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

namespace NanoboyAdvance::GBA {

class CPU : private ARM::Interface {
public:
    CPU(Config* config);

    auto GetConfig() -> Config* const;
    void SetConfig(Config* config);

    void Reset();
    void SetSlot1(uint8_t* rom, size_t size);

    void RunFor(int cycles);

    enum class HaltControl {
        RUN,
        STOP,
        HALT
    };

    enum class Interrupt {
        VBLANK  = 1 << 0,
        HBLANK  = 1 << 1,
        VCOUNT  = 1 << 2,
        TIMER0  = 1 << 3,
        TIMER1  = 1 << 4,
        TIMER2  = 1 << 5,
        TIMER3  = 1 << 6,
        SERIAL  = 1 << 7,
        DMA0    = 1 << 8,
        DMA1    = 1 << 9,
        DMA2    = 1 << 10,
        DMA3    = 1 << 11,
        KEYPAD  = 1 << 12,
        GAMEPAK = 1 << 13
    };

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

    struct MMIO {
        std::uint16_t irq_ie;
        std::uint16_t irq_if;
        std::uint16_t irq_ime;

        HaltControl haltcnt;

        struct WaitstateControl {
            int sram;
            int ws0_n;
            int ws0_s;
            int ws1_n;
            int ws1_s;
            int ws2_n;
            int ws2_s;
            int phi;
            int prefetch;
            int cgb;
        } waitcnt;
    } mmio;

private:
    #include "memory.inl"

    void Tick(int cycles) final { }
    void SWI(std::uint32_t call_id) final { }

    auto ReadMMIO(std::uint32_t address) -> std::uint8_t;
    void WriteMMIO(std::uint32_t address, std::uint8_t value);

    void UpdateCycleLUT();

    ARM::ARM7 cpu;
    PPU ppu;
    Config* config;

    int run_until = 0;
    int cycles16[2][16];
    int cycles32[2][16];

    static constexpr int s_ws_nseq[4] = { 4, 3, 2, 8 }; /* Non-sequential SRAM/WS0/WS1/WS2 */
    static constexpr int s_ws_seq0[2] = { 2, 1 };       /* Sequential WS0 */
    static constexpr int s_ws_seq1[2] = { 4, 1 };       /* Sequential WS1 */
    static constexpr int s_ws_seq2[2] = { 8, 1 };       /* Sequential WS2 */
};

}
