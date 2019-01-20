/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../cpu.hpp"
#include "ppu.hpp"

using namespace NanoboyAdvance::GBA;

void PPU::DecodeTile4bpp(std::uint16_t* buffer, std::uint32_t base, int palette, int number, int y, bool flip) {
    std::uint8_t* data = &cpu->memory.vram[base + (number * 32) + (y * 4)];

    if (flip) {
        for (int x = 0; x < 4; x++) {
            int d  = *data++;
            int p1 = d & 15;
            int p2 = d >> 4;

            buffer[(x*2+0)^7] = p1 ? ReadPalette(palette, p1) : 0x8000;
            buffer[(x*2+1)^7] = p2 ? ReadPalette(palette, p2) : 0x8000;
        }
    } else {
        for (int x = 0; x < 4; x++) {
            int d  = *data++;
            int p1 = d & 15;
            int p2 = d >> 4;

            buffer[x*2+0] = p1 ? ReadPalette(palette, p1) : 0x8000;
            buffer[x*2+1] = p2 ? ReadPalette(palette, p2) : 0x8000;
        }
    }
}

void PPU::DrawPixel(int x, int priority, std::uint16_t color) {
    if (color != s_color_transparent && priority <= this->priority[x]) {
        pixel[1][x] = pixel[0][x];
        pixel[0][x] = color;
        this->priority[x] = priority;
    }
}

void PPU::RenderText(int id) {
    const auto& bgcnt = mmio.bgcnt[id];

    int last_encoder = -1;
    std::uint16_t tile[8];
    std::uint32_t tile_base = bgcnt.tile_block * 16384;
    
    int line = mmio.vcount + mmio.bgvofs[id];
    int row  = line / 8;
    int tile_y = line % 8;

    int screen_y = (row / 32) % 2;

    std::uint32_t base = (bgcnt.map_block * 2048) + ((row % 32) * 64);

    int draw_x = -(mmio.bghofs[id] % 8);
    int column =   mmio.bghofs[id] / 8;

    while (draw_x < 240) {
        int screen_x = (column / 32) % 2;
        std::uint32_t offset = base + ((column++ % 32) * 2);

        switch (bgcnt.size) {
            case 1: offset +=  screen_x * 2048; break;
            case 2: offset +=  screen_y * 2048; break;
            case 3: offset += (screen_x * 2048) + (screen_y * 4096); break;
        }

        std::uint16_t encoder = (cpu->memory.vram[offset + 1] << 8) | cpu->memory.vram[offset];

        if (encoder != last_encoder) {
            int number  = encoder & 0x3FF;
            int palette = encoder >> 12;
            bool flip_x = encoder & (1 << 10);
            bool flip_y = encoder & (1 << 11);
            int _tile_y = flip_y ? (tile_y ^ 7) : tile_y;

            if (!bgcnt.full_palette) {
                DecodeTile4bpp(tile, tile_base, palette, number, _tile_y, flip_x);
            } else {

            }

            last_encoder = encoder;
        }

        if (draw_x >= 0 & draw_x <= 232) {
            for (int x = 0; x < 8; x++) {
                DrawPixel(draw_x++, bgcnt.priority, tile[x]);
            }
        } else {
            for (int x = 0; x < 8; x++) {
                if (draw_x >= 0 && draw_x < 240) {
                    DrawPixel(draw_x, bgcnt.priority, tile[x]);
                }
                draw_x++;
            }
        }
    }
}
