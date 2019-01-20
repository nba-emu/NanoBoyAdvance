/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstdint>

namespace NanoboyAdvance::GBA {

struct DisplayControl {
    int mode;
    int cgb_mode;
    int frame;
    int hblank_oam_access;
    int oam_mapping_1d;
    int forced_blank;
    int enable[8];

    void Reset();
    auto Read(int address) -> std::uint8_t;
    void Write(int address, std::uint8_t value);
};

struct DisplayStatus {
    int vblank_flag;
    int hblank_flag;
    int vcount_flag;
    int vblank_irq_enable;
    int hblank_irq_enable;
    int vcount_irq_enable;
    int vcount_setting;

    void Reset();
    auto Read(int address) -> std::uint8_t;
    void Write(int address, std::uint8_t value);
};

struct BackgroundControl {
    int priority;
    int tile_block;
    int mosaic_enable;
    int full_palette;
    int map_block;
    int wraparound;
    int size;

    void Reset();
    auto Read(int address) -> std::uint8_t;
    void Write(int address, std::uint8_t value);
};

struct BlendControl {
    enum Effect {
        SFX_NONE,
        SFX_BLEND,
        SFX_BRIGHTEN,
        SFX_DARKEN
    } sfx;
    
    int targets[2][6];

    void Reset();
    auto Read(int address) -> std::uint8_t;
    void Write(int address, std::uint8_t value);
};

}
