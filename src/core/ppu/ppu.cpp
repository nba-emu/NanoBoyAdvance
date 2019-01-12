/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ppu.hpp"

namespace NanoboyAdvance {
namespace GBA {

const int PPU::s_wait_cycles[3] = { 960, 272, 1232 };

void PPU::Reset() {
    mmio.dispcnt.Reset();
    mmio.dispstat.Reset();
    mmio.vcount = 0;
    wait_cycles = s_wait_cycles[PHASE_SCANLINE];
}

void PPU::Tick() {
    auto& vcount = mmio.vcount;
    auto& dispstat = mmio.dispstat;

    switch (phase) {
    case PHASE_SCANLINE:
        phase = PHASE_HBLANK;
        wait_cycles = s_wait_cycles[PHASE_HBLANK];
        dispstat.hblank_flag = 1;
        break;
    case PHASE_HBLANK:
        dispstat.hblank_flag = 0;
        dispstat.vcount_flag = ++vcount == dispstat.vcount_setting;

        if (vcount == 160) {
            dispstat.vblank_flag = 1;
            phase = PHASE_VBLANK;
            wait_cycles = s_wait_cycles[PHASE_VBLANK];
        } else {
            phase = PHASE_SCANLINE;
            wait_cycles = s_wait_cycles[PHASE_SCANLINE];
        }
        break;
    case PHASE_VBLANK:
        if (vcount == 227) {
            dispstat.vblank_flag = 0;
            phase = PHASE_SCANLINE;
            wait_cycles = s_wait_cycles[PHASE_SCANLINE];

            /* Update vertical counter. */
            vcount = 0;
            dispstat.vcount_flag = dispstat.vcount_setting == 0;
        } else {
            wait_cycles = s_wait_cycles[PHASE_VBLANK];
            
            /* Update vertical counter. */
            dispstat.vcount_flag = ++vcount == dispstat.vcount_setting;
        }
        break;
    }
}

} // namespace GBA
} // namespace NanoboyAdvance