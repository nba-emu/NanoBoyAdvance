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
    void ppu::render_rotate_scale(int id)
    {
        auto bg        = m_io.bgcnt[2 + id];
        u16* buffer    = m_buffer[2 + id];
        u32 tile_block = bg.tile_block << 14;

        float ref_x = m_io.bgx[id].internal;
        float ref_y = m_io.bgy[id].internal;
        float parameter_a = ppu::decode_float16(m_io.bgpa[id]);
        float parameter_c = ppu::decode_float16(m_io.bgpc[id]);

        int size, block_width;

        switch (bg.screen_size) // optimize
        {
        case 0: size = 128; block_width = 16; break;
        case 1: size = 256; block_width = 32; break;
        case 2: size = 512; block_width = 64; break;
        case 3: size = 1024; block_width = 128; break;
        }

        for (int _x = 0; _x < 240; _x++)
        {
            bool is_backdrop = false;

            int x = ref_x + _x * parameter_a;
            int y = ref_y + _x * parameter_c;

            if (x >= size)
            {
                if (bg.wraparound) x = x % size;
                else is_backdrop = true;
            }
            else if (x < 0)
            {
                if (bg.wraparound) x = (size + x) % size;
                else is_backdrop = true;
            }

            if (y >= size)
            {
                if (bg.wraparound) y = y % size;
                else is_backdrop = true;
            }
            else if (y < 0)
            {
                if (bg.wraparound) y = (size + y) % size;
                else is_backdrop = true;
            }

            if (is_backdrop)
            {
                buffer[_x] = COLOR_TRANSPARENT;
                continue;
            }

            int map_x  = x >> 3;
            int map_y  = y >> 3;
            int tile_x = x & 7;
            int tile_y = y & 7;

            int number = m_vram[(bg.map_block << 11) + map_y * block_width + map_x];

            buffer[_x] = get_tile_pixel_8bpp(tile_block, number, tile_x, tile_y);
        }
    }
}
