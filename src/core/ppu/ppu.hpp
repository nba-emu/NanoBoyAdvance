/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstdint>
#include "../config.hpp"
#include "io.hpp"

namespace NanoboyAdvance {
namespace GBA {

class CPU;

class PPU {
public:
    PPU(Config* config, CPU* cpu);

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
    static auto ConvertColor(std::uint16_t color) -> std::uint32_t;
    auto ReadPalette(int palette, int index) -> std::uint16_t;
    void RenderScanline();

    CPU* cpu;
    Config* config;
    std::uint8_t* pram;
    std::uint8_t* vram;
    std::uint8_t* oam;

    enum Phase {
        PHASE_SCANLINE = 0,
        PHASE_HBLANK = 1,
        PHASE_VBLANK = 2
    };

    enum Phase phase;

    static const int s_wait_cycles[3];
};

} // namespace GBA
} // namespace NanoboyAdvance
