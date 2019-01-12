/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "io.hpp"

namespace NanoboyAdvance {
namespace GBA {

class PPU {
    enum Phase {
        PHASE_SCANLINE = 0,
        PHASE_HBLANK = 1,
        PHASE_VBLANK = 2
    };

    static const int s_wait_cycles[3];

    enum Phase phase;
public:
    int wait_cycles;

    struct MMIO {
        DisplayControl dispcnt;
        DisplayStatus dispstat;
        std::uint8_t vcount;
    } mmio;

    void Reset();
    void Tick();
};

} // namespace GBA
} // namespace NanoboyAdvance
