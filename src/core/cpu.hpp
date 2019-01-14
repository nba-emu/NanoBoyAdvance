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

namespace NanoboyAdvance {
namespace GBA {

class CPU : private ARM::Interface {
public:
    CPU(Config* config);

    auto GetConfig() -> Config* const;
    void SetConfig(Config* config);

    void Reset();
    void SetSlot1(uint8_t* rom, size_t size);

    void RunFor(int cycles);

    enum HaltControl {
        SYSTEM_RUN,
        SYSTEM_STOP,
        SYSTEM_HALT
    };

    enum Interrupt {
        INT_VBLANK  = 1 << 0,
        INT_HBLANK  = 1 << 1,
        INT_VCOUNT  = 1 << 2,
        INT_TIMER0  = 1 << 3,
        INT_TIMER1  = 1 << 4,
        INT_TIMER2  = 1 << 5,
        INT_TIMER3  = 1 << 6,
        INT_SERIAL  = 1 << 7,
        INT_DMA0    = 1 << 8,
        INT_DMA1    = 1 << 9,
        INT_DMA2    = 1 << 10,
        INT_DMA3    = 1 << 11,
        INT_KEYPAD  = 1 << 12,
        INT_GAMEPAK = 1 << 13
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
    } mmio;

private:
    #include "memory.inl"

    void Tick(int cycles) final { }
    void SWI(std::uint32_t call_id) final { }

    auto ReadMMIO(std::uint32_t address) -> std::uint8_t;
    void WriteMMIO(std::uint32_t address, std::uint8_t value);

    ARM::ARM7 cpu;
    PPU ppu;
    Config* config;
};

} // namespace GBA
} // namespace NanoboyAdvance
