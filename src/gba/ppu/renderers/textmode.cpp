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
        auto bg        = m_io.bgcnt[id];
        u16* buffer    = m_buffer[id];
        u32 tile_block = bg.tile_block << 14;

        int line     = m_io.vcount + m_io.bgvofs[id];
        int row      = line >> 3;
        int tile_y   = line & 7;
        int screen_y = (row >> 5) & 1;

        u8 tile_buffer[8];
        int palette, number;
        bool h_flip, v_flip;
        int last_number       = -1;
        int last_tile_encoder = -1;

        u32 base_offset = (bg.map_block << 11) + ((row & 0x1F) << 6);

        for (int _x = 0; _x < 240; _x++)
        {
            int x = _x + m_io.bghofs[id];

            int col     = x >> 3;
            int  tile_x = x & 7;
            int _tile_y = tile_y;

            int screen_x = (col >> 5) & 1;
            u32 offset   = base_offset + ((col & 0x1F) << 1);

            switch (bg.screen_size)
            {
            case 1: offset += screen_x << 11; break;
            case 2: offset += screen_y << 11; break;
            case 3: offset += (screen_x << 11) + (screen_y << 12); break;
            }

            u16 tile_encoder = (m_vram[offset + 1] << 8) | m_vram[offset];

            if (tile_encoder != last_tile_encoder)
            {
                number  = tile_encoder & 0x3FF;
                h_flip  = tile_encoder & (1 << 10);
                v_flip  = tile_encoder & (1 << 11);
                palette = tile_encoder >> 12;
                last_tile_encoder = tile_encoder;
            }

            if (h_flip)  tile_x ^= 7;
            if (v_flip) _tile_y ^= 7;

            if (is_256)
            {
                buffer[_x] = get_tile_pixel_8bpp(tile_block, number, tile_x, _tile_y);
            }
            else
            {
                buffer[_x] = get_tile_pixel_4bpp(tile_block, palette, number, tile_x, _tile_y);
            }
        }
    }
}
