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
#include "util/logger.hpp"

using namespace util;

namespace gba
{
    void ppu::render_textmode(int id)
    {
        if (m_io.bgcnt[id].full_palette)
        {
            switch (id)
            {
            case 0: render_textmode_internal<true, 0>(); return;
            case 1: render_textmode_internal<true, 1>(); return;
            case 2: render_textmode_internal<true, 2>(); return;
            case 3: render_textmode_internal<true, 3>(); return;
            }
        }
        else
        {
            switch (id)
            {
            case 0: render_textmode_internal<false, 0>(); return;
            case 1: render_textmode_internal<false, 1>(); return;
            case 2: render_textmode_internal<false, 2>(); return;
            case 3: render_textmode_internal<false, 3>(); return;
            }
        }
    }

    template <bool is_256, int id>
    void ppu::render_textmode_internal()
    {
        auto bg = m_io.bgcnt[id];
        u16* buffer = m_buffer[id];
        u32 tile_block = bg.tile_block << 14;

        bool h_expand = bg.screen_size & 1;
        bool v_expand = bg.screen_size & 3;

        int line   = (m_io.vcount + m_io.bgvofs[id]) & (v_expand ? 0x1FF : 0xFF);
        int row    = line >> 3;
        int tile_y = line & 7;

        u32 offset = (bg.map_block << 11) + (row << (h_expand ? 7 : 6));

        if (v_expand) offset += h_expand ? 0x1600 : 0x800;

        int palette, number;
        bool h_flip, v_flip;
        int last_number = -1;
        int last_tile_encoder = -1;
        u8 tile_buffer[8];

        for (int _x = 0; _x < 240; _x++)
        {
            int x = (_x + m_io.bghofs[id]) & (h_expand ? 0x1FF : 0xFF);

            int col    = x >> 3;
            int tile_x = x & 7;

            u32 tile_offset  = offset + (col << 1);
            u16 tile_encoder = (m_vram[tile_offset + 1] << 8) | m_vram[tile_offset];
            int _tile_y = tile_y;

            if (tile_encoder != last_tile_encoder)
            {
                number  = tile_encoder & 0x3FF;
                h_flip  = tile_encoder & (1 << 10);
                v_flip  = tile_encoder & (1 << 11);
                palette = tile_encoder >> 12;
                last_tile_encoder = tile_encoder;
            }

            if (h_flip) tile_x ^= 7;
            if (v_flip) _tile_y ^= 7;

            if (is_256)
            {
                if (number != last_number)
                {
                    get_tile_line_8bpp(tile_buffer, bg.tile_block * 0x4000, number, _tile_y);
                    last_number = number;
                }

                int index = tile_buffer[tile_x];

                if (index == 0)
                {
                    buffer[_x] = COLOR_TRANSPARENT;
                    continue;
                }

                buffer[_x] = (m_pal[(index<<1)+1] << 8) | m_pal[index<<1];
            }
            else
            {
                if (number != last_number)
                {
                    get_tile_line_4bpp(tile_buffer, bg.tile_block * 0x4000, number, _tile_y);
                    last_number = number;
                }

                int index = tile_buffer[tile_x];

                if (index == 0)
                {
                    buffer[_x] = COLOR_TRANSPARENT;
                    continue;
                }

                buffer[_x] = (m_pal[(palette<<5)+(index<<1)+1] << 8) | m_pal[(palette<<5)+(index<<1)];
            }
        }
    }
}
