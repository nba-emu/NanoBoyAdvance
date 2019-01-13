/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstdint>
#include "io.hpp"
#include "../config.hpp"

namespace NanoboyAdvance {
namespace GBA {

class CPU;

class PPU {
    friend class CPU;

    int wait_cycles;

    enum Phase {
        PHASE_SCANLINE = 0,
        PHASE_HBLANK = 1,
        PHASE_VBLANK = 2
    };

    enum Phase phase; 

    struct MMIO {
        DisplayControl dispcnt;
        DisplayStatus dispstat;

        std::uint8_t vcount;
        
        BackgroundControl bgcnt[4];
        std::uint16_t bghofs[4];
        std::uint16_t bgvofs[4];
    } mmio;

    Config* config;

    std::uint8_t* pram;
    std::uint8_t* vram;
    std::uint8_t* oam;

    static const int s_wait_cycles[3];

    static auto ConvertColor(std::uint16_t color) -> std::uint32_t;
    auto ReadPalette(int palette, int index) -> std::uint16_t;
public:
    void Reset();
    void Tick();
    void RenderScanline();
};

} // namespace GBA
} // namespace NanoboyAdvance
