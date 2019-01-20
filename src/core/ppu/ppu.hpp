/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstdint>

#include "../config.hpp"
#include "regs.hpp"

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

        BlendControl bldcnt;
        int eva;
        int evb;
        int evy;
    } mmio;

    int wait_cycles;

private:
    enum class Phase {
        SCANLINE = 0,
        HBLANK = 1,
        VBLANK = 2
    };

    enum ObjAttribute {
        OBJ_IS_ALPHA  = 1,
        OBJ_IS_WINDOW = 2
    };

    enum ObjectMode {
        OBJ_NORMAL     = 0,
        OBJ_SEMI       = 1,
        OBJ_WINDOW     = 2,
        OBJ_PROHIBITED = 3
    };

    static auto ConvertColor(std::uint16_t color) -> std::uint32_t;
    void InitBlendTable();
    void Next(Phase phase);
    void RenderScanline();
    void RenderLayerText(int id);
    void RenderLayerOAM();
    void Blend(std::uint16_t* target1, std::uint16_t target2, BlendControl::Effect sfx);

    #include "ppu.inl"

    CPU* cpu;

    Config* config;
    std::uint8_t* pram;
    std::uint8_t* vram;
    std::uint8_t* oam;

    std::uint8_t  obj_attr[240];
    std::uint8_t  priority[240];
    std::uint8_t  layer[2][240];
    std::uint16_t pixel[2][240];

    Phase phase;

    std::uint8_t blend_table[17][17][32][32];

    static constexpr std::uint16_t s_color_transparent = 0x8000;
    static constexpr int s_wait_cycles[3] = { 960, 272, 1232 };
    static const int s_obj_size[4][4][2];
};

}
