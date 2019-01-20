/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstdint>

#include "io.hpp"

namespace NanoboyAdvance::GBA {

class CPU;

class PPU {
public:
    PPU(CPU* cpu);

    void Reset();
    void Tick();

    struct MMIO {
        DisplayControl dispcnt;
        DisplayStatus dispstat;

        std::uint8_t vcount;
        
        BackgroundControl bgcnt[4];
        std::uint16_t bghofs[4];
        std::uint16_t bgvofs[4];
    } mmio;

    int wait_cycles;

private:
    enum class Phase {
        SCANLINE = 0,
        HBLANK = 1,
        VBLANK = 2
    };

    static auto ConvertColor(std::uint16_t color) -> std::uint32_t;
    void Next(Phase phase);
    void RenderScanline();
    void RenderText(int id);

    #include "ppu.inl"

    CPU* cpu;

    Config* config;
    std::uint8_t* pram;
    std::uint8_t* vram;
    std::uint8_t* oam;

    std::uint8_t  priority[240];
    std::uint8_t  layer[2][240];
    std::uint16_t pixel[2][240];

    Phase phase;

    static constexpr std::uint16_t s_color_transparent = 0x8000;
    static constexpr int s_wait_cycles[3] = { 960, 272, 1232 };
};

}
