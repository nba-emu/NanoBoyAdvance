///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2016 Frederic Meyer
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


#include "../video.h"


namespace GBA
{
    void Video::RenderTextModeBG(int id)
    {
        struct Background bg = this->m_BG[id];
        u16* buffer = m_BgBuffer[id];

        int width = ((bg.size & 1) + 1) * 256;
        int height = ((bg.size >> 1) + 1) * 256;
        int y_scrolled = (m_VCount + bg.y) % height;
        int row = y_scrolled / 8;
        int row_rmdr = y_scrolled % 8;
        int left_area = 0;
        int right_area = 1;
        u16 line_buffer[width];
        u32 offset;

        if (row >= 32)
        {
            left_area = (bg.size & 1) + 1;
            right_area = 3;
            row -= 32;
        }

        offset = bg.map_base + left_area * 0x800 + 64 * row;

        for (int x = 0; x < width / 8; x++)
        {
            u16 tile_encoder = (m_VRAM[offset + 1] << 8) |
                                m_VRAM[offset];
            int tile_number = tile_encoder & 0x3FF;
            bool horizontal_flip = tile_encoder & (1 << 10);
            bool vertical_flip = tile_encoder & (1 << 11);
            int row_rmdr_flip = row_rmdr;
            u16* tile_data;

            if (vertical_flip)
                row_rmdr_flip = 7 - row_rmdr_flip;

            if (bg.true_color)
            {
                tile_data = DecodeTileLine8BPP(bg.tile_base, tile_number, row_rmdr_flip, false);
            } else
            {
                int palette = tile_encoder >> 12;
                tile_data = DecodeTileLine4BPP(bg.tile_base, palette * 0x20, tile_number, row_rmdr_flip);
            }

            if (horizontal_flip)
                for (int i = 0; i < 8; i++)
                    line_buffer[x * 8 + i] = tile_data[7 - i];
            else
                for (int i = 0; i < 8; i++)
                    line_buffer[x * 8 + i] = tile_data[i];

            delete[] tile_data;

            if (x == 31)
                offset = bg.map_base + right_area * 0x800 + 64 * row;
            else
                offset += 2;
        }

        for (int i = 0; i < 240; i++)
            buffer[i] = line_buffer[(bg.x + i) % width];
    }
}
