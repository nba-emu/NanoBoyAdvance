/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstring>

#include "../cpu.hpp"
#include "ppu.hpp"

using namespace NanoboyAdvance::GBA;

constexpr std::uint16_t PPU::s_color_transparent;
constexpr int PPU::s_wait_cycles[3];

PPU::PPU(CPU* cpu) 
    : cpu(cpu)
    , config(cpu->config)
    , pram(cpu->memory.pram)
    , vram(cpu->memory.vram)
    , oam(cpu->memory.pram)
{
    Reset();
}

void PPU::Reset() {
    mmio.dispcnt.Reset();
    mmio.dispstat.Reset();
    mmio.vcount = 0;

    for (int i = 0; i < 4; i++) {
        mmio.bgcnt[i].Reset();
        mmio.bghofs[i] = 0;
        mmio.bgvofs[i] = 0;
    }
    
    wait_cycles = 0;
    Next(Phase::SCANLINE);
    //RenderScanline();
}

void PPU::Next(Phase phase) {
    this->phase  = phase;
    wait_cycles += s_wait_cycles[static_cast<int>(phase)];
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

void PPU::Tick() {
    auto& vcount = mmio.vcount;
    auto& dispstat = mmio.dispstat;

    switch (phase) {
        case Phase::SCANLINE: {
            Next(Phase::HBLANK);
            dispstat.hblank_flag = 1;

            if (dispstat.hblank_irq_enable) {
                cpu->mmio.irq_if |= CPU::INT_HBLANK;
            }
            break;
        }
        case Phase::HBLANK: {
            dispstat.hblank_flag = 0;
            dispstat.vcount_flag = ++vcount == dispstat.vcount_setting;

            if (dispstat.vcount_flag && dispstat.vcount_irq_enable) {
                cpu->mmio.irq_if |= CPU::INT_VCOUNT;
            }

            if (vcount == 160) {
                dispstat.vblank_flag = 1;
                Next(Phase::VBLANK);

                if (dispstat.vblank_irq_enable) {
                    cpu->mmio.irq_if |= CPU::INT_VBLANK;
                }
            } else {
                Next(Phase::SCANLINE);
                RenderScanline();
            }
            break;
        }
        case Phase::VBLANK: {
            if (vcount == 227) {
                Next(Phase::SCANLINE);

                /* Update vertical counter. */
                vcount = 0;
                dispstat.vcount_flag = dispstat.vcount_setting == 0;

                RenderScanline();
            } else {
                Next(Phase::VBLANK);

                if (vcount == 226)
                    dispstat.vblank_flag = 0;
                
                /* Update vertical counter. */
                dispstat.vcount_flag = ++vcount == dispstat.vcount_setting;
            }

            if (dispstat.vcount_flag && dispstat.vcount_irq_enable) {
                cpu->mmio.irq_if |= CPU::INT_VCOUNT;
            }
            break;
        }
    }
}

void PPU::RenderScanline() {
    std::uint16_t vcount = mmio.vcount;
    std::uint32_t* line = &config->video.output[vcount * 240];

    if (mmio.dispcnt.forced_blank) {
        for (int x = 0; x < 240; x++)
            line[x] = ConvertColor(0x7FFF);
    } else {
        std::uint16_t backdrop = ReadPalette(0, 0);
        std::uint32_t fill = backdrop * 0x00010001;

        for (int i = 0; i < 240; i++)
            ((std::uint32_t*)pixel)[i] = fill;
        for (int i = 0; i < 60; i++)
            ((std::uint32_t*)priority)[i] = 0x04040404;
        for (int i = 0; i < 120; i++)
            ((std::uint32_t*)layer)[i] = 0x05050505;
        
        /* TODO: how does HW behave when we select mode 6 or 7? */
        switch (mmio.dispcnt.mode) {
            case 0: {
                for (int i = 3; i >= 0; i--) {
                    if (mmio.dispcnt.enable[i])
                        RenderLayerText(i);
                }
                for (int x = 0; x < 240; x++) {
                    line[x] = ConvertColor(pixel[0][x]);
                }
                break;
            }
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
                    line[x] = ConvertColor(ReadPalette(0, cpu->memory.vram[offset + x]));
                }
                break;
            }
            case 5:
                break;
        }
    }
}
