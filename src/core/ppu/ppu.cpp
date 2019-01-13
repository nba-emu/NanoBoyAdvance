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
    
    for (int i = 0; i < 4; i++) {
        mmio.bgcnt[i].Reset();
        mmio.bghofs[i] = 0;
        mmio.bgvofs[i] = 0;
    }
    
    wait_cycles = s_wait_cycles[PHASE_SCANLINE];
}

auto PPU::ConvertColor(std::uint16_t color) -> std::uint32_t {
    int r = (color >>  0) & 0x1F;
    int g = (color >>  5) & 0x1F;
    int b = (color >> 10) & 0x1F;

    return r << 19 |
           g << 11 |
           b <<  3 |
           0xFF000000;
}

auto PPU::ReadPalette(int palette, int index) -> std::uint16_t {
    int cell = (palette * 32) + (index * 2);

    /* TODO: On Little-Endian devices we can get away with casting to uint16_t*. */
    return (pram[cell + 1] << 8) |
            pram[cell + 0];
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
            RenderScanline();
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

            RenderScanline();
        } else {
            wait_cycles = s_wait_cycles[PHASE_VBLANK];
            
            /* Update vertical counter. */
            dispstat.vcount_flag = ++vcount == dispstat.vcount_setting;
        }
        break;
    }
}

void PPU::RenderScanline() {
    std::uint16_t vcount = mmio.vcount;
    std::uint32_t* line = &config->video.output[vcount * 240];

    if (mmio.dispcnt.forced_blank) {
        for (int x = 0; x < 240; x++)
            line[x] = ConvertColor(0x7FFF);
    } else {
        /* TODO: how does HW behave when we select mode 6 or 7? */
        switch (mmio.dispcnt.mode) {
            case 0:
                break;
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
            case 4: {
                int frame = mmio.dispcnt.frame * 0xA000;
                int offset = frame + vcount * 240;
                for (int x = 0; x < 240; x++) {
                    line[x] = ConvertColor(ReadPalette(0, vram[offset + x]));
                }
                break;
            }
            case 5:
                break;
        }
    }
}

} // namespace GBA
} // namespace NanoboyAdvance