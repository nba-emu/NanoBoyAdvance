/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#include "../ppu.hpp"
#include "util/likely.hpp"

namespace Core {

    void PPU::renderTextBG(int id) {
        const auto& bg = regs.bgcnt[id];

        u16* buffer    = m_buffer[id];
        u32 tile_block = bg.tile_block << 14;

        u32 tile_buffer[8];
        int last_encoder = -1;

        // scrolled scanline
        const int line = regs.vcount + regs.bgvofs[id];

        // vertical position data
        const int row      = line >> 3;
        const int tile_y   = line &  7;
        const int screen_y = (row >> 5) & 1;

        // points to the start of the row's tile map information
        const u32 base_offset = (bg.map_block << 11) + ((row & 0x1F) << 6);

        // current pixel being drawn, current map column
        int draw_x = -(regs.bghofs[id]  & 7);
        int column =   regs.bghofs[id] >> 3;

        while (draw_x < 240) {
            int screen_x = (column >> 5) & 1;

            int offset = base_offset + ((column++ & 0x1F) << 1);

            switch (bg.screen_size) {
                case 1: offset += screen_x << 11; break;
                case 2: offset += screen_y << 11; break;
                case 3: offset += (screen_x << 11) + (screen_y << 12); break;
            }

            u16 encoder = (m_vram[offset + 1] << 8) | m_vram[offset];

            // only decode a new tile if neccessary
            if (encoder != last_encoder) {
                // tile number and palette
                const int number  = encoder & 0x3FF;
                const int palette = encoder >> 12;

                // flipping information
                const bool h_flip = encoder & (1 << 10);
                const bool v_flip = encoder & (1 << 11);

                const int final_y = v_flip ? (tile_y ^ 7) : tile_y;

                // decode the determined tile into our buffer
                if (UNLIKELY(bg.full_palette)) {
                    drawTileLine8BPP(tile_buffer, tile_block, number, final_y, h_flip);
                } else {
                    drawTileLine4BPP(tile_buffer, tile_block, palette, number, final_y, h_flip);
                }

                last_encoder = encoder;
            }

            if (LIKELY(draw_x >= 0 && draw_x <= 232)) {
                for (int x = 0; x < 8; x++) {
                    buffer[draw_x++] = tile_buffer[x];
                }
            } else {
                for (int x = 0; x < 8; x++) {
                    if (draw_x < 0 || draw_x >= 240) {
                        draw_x++;
                        continue;
                    }
                    buffer[draw_x++] = tile_buffer[x];
                }
            }
        }
    }
}
