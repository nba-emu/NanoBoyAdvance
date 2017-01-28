///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2017 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////

#include "../ppu.hpp"

namespace gba
{
    void ppu::render_textmode(int id)
    {
        u16* buffer = m_buffer[id];
        auto bg = m_io.bgcnt[id];

        // background dimensions
        int width = ((bg.screen_size & 1) + 1) << 8;
        int height = ((bg.screen_size >> 1) + 1) << 8;

        int line = (m_io.vcount + m_io.bgvofs[id]) % height;
        int row = line / 8;
        int row_rmdr = line % 8;
        int left_area = 0;
        int right_area = 1;
        u16 tile_buffer[8];
        u16 line_buffer[width];
        u32 offset;

        if (row >= 32)
        {
            left_area = (bg.screen_size & 1) + 1;
            right_area = 3;
            row -= 32;
        }

        offset = bg.map_block * 0x800 + left_area * 0x800 + 64 * row;

        for (int x = 0; x < width / 8; x++)
        {
            u16 tile_encoder = (m_vram[offset + 1] << 8) | m_vram[offset];
            int tile_number = tile_encoder & 0x3FF;
            bool horizontal_flip = tile_encoder & (1 << 10);
            bool vertical_flip = tile_encoder & (1 << 11);
            int row_rmdr_flip = row_rmdr;

            if (vertical_flip)
                row_rmdr_flip = 7 - row_rmdr_flip;

            if (bg.full_palette)
            {
                decode_tile_line_8bpp(tile_buffer, bg.tile_block * 0x4000, tile_number, row_rmdr_flip, false);
            }
            else
            {
                int palette = tile_encoder >> 12;
                decode_tile_line_4bpp(tile_buffer, bg.tile_block * 0x4000, palette * 0x20, tile_number, row_rmdr_flip);
            }

            if (horizontal_flip)
            {
                for (int i = 0; i < 8; i++)
                    line_buffer[x * 8 + i] = tile_buffer[7 - i];
            }
            else
            {
                for (int i = 0; i < 8; i++)
                    line_buffer[x * 8 + i] = tile_buffer[i];
            }

            if (x == 31)
                offset = bg.map_block * 0x800 + right_area * 0x800 + 64 * row;
            else
                offset += 2;
        }

        for (int i = 0; i < 240; i++)
            buffer[i] = line_buffer[(m_io.bghofs[id] + i) % width];
    }
}
