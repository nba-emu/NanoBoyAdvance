/*
 * Copyright (C) 2018 Frederic Meyer. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../ppu.hpp"

using namespace GameBoyAdvance;

void PPU::RenderLayerText(int id) {
    const auto& bgcnt = mmio.bgcnt[id];
    
    std::uint32_t tile_base = bgcnt.tile_block * 16384;
   
    int line = mmio.bgvofs[id] + mmio.vcount;

    int draw_x = -(mmio.bghofs[id] % 8);
    int grid_x =   mmio.bghofs[id] / 8;
    int grid_y = line / 8;
    int tile_y = line % 8;
    
    int screen_x;
    int screen_y = (grid_y / 32) % 2;

    std::uint16_t tile[8];
    std::int32_t  last_encoder = -1;
    std::uint16_t encoder;
    std::uint32_t offset;
    std::uint32_t base = (bgcnt.map_block * 2048) + ((grid_y % 32) * 64);

    while (draw_x < 240) {
        screen_x = (grid_x / 32) % 2;
        offset = base + ((grid_x++ % 32) * 2);

        switch (bgcnt.size) {
            case 1: offset +=  screen_x * 2048; break;
            case 2: offset +=  screen_y * 2048; break;
            case 3: offset += (screen_x * 2048) + (screen_y * 4096); break;
        }

        encoder = (vram[offset + 1] << 8) | vram[offset];

        if (encoder != last_encoder) {
            int number  = encoder & 0x3FF;
            int palette = encoder >> 12;
            bool flip_x = encoder & (1 << 10);
            bool flip_y = encoder & (1 << 11);
            int _tile_y = flip_y ? (tile_y ^ 7) : tile_y;

            if (!bgcnt.full_palette) {
                DecodeTileLine4BPP(tile, tile_base, palette, number, _tile_y, flip_x);
            } else {
                DecodeTileLine8BPP(tile, tile_base, number, _tile_y, flip_x);
            }

            last_encoder = encoder;
        }

        if (draw_x >= 0 && draw_x <= 232) {
            for (int x = 0; x < 8; x++) {
                DrawPixel(draw_x++, id, bgcnt.priority, tile[x]);
            }
        } else {
            int x = 0;
            int max = 8;

            if (draw_x < 0) {
                x = -draw_x;
                draw_x = 0;
            }
            if (draw_x > 232) {
                max -= draw_x - 232;
            }

            for (x; x < max; x++) {
                DrawPixel(draw_x++, id, bgcnt.priority, tile[x]);
            }
        }
    }
}
