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

namespace gba
{
    static constexpr int g_sprite_size[4][4][2] = {
        /* SQUARE */
        {
            { 8 , 8  },
            { 16, 16 },
            { 32, 32 },
            { 64, 64 }
        },
        /* HORIZONTAL */
        {
            { 16, 8  },
            { 32, 8  },
            { 32, 16 },
            { 64, 32 }
        },
        /* VERTICAL */
        {
            { 8 , 16 },
            { 8 , 32 },
            { 16, 32 },
            { 32, 64 }
        },
        /* PROHIBITED */
        {
            { 0, 0 },
            { 0, 0 },
            { 0, 0 },
            { 0, 0 }
        }
    };

    void ppu::render_sprites(u32 tile_base)
    {
        u8  tile_buffer[8];
        u32 offset = 127 << 3;

        // eh...
        for (int i = 0; i < 240; i++)
        {
            m_buffer[4][i] = COLOR_TRANSPARENT;
            m_buffer[5][i] = COLOR_TRANSPARENT;
            m_buffer[6][i] = COLOR_TRANSPARENT;
            m_buffer[7][i] = COLOR_TRANSPARENT;
        }

        // TODO(performance):
        // we have to read OAM data in descending order but that
        // might affect dcache performance? not sure about impact.
        for (int i = 0; i < 128; i++)
        {
            // TODO(performance): decode these on OAM writes already?
            u16 attribute0 = (m_oam[offset + 1] << 8) | m_oam[offset + 0];
            u16 attribute1 = (m_oam[offset + 3] << 8) | m_oam[offset + 2];
            u16 attribute2 = (m_oam[offset + 5] << 8) | m_oam[offset + 4];

            int width, height;
            int x = attribute1 & 0x1FF;
            int y = attribute0 & 0x0FF;
            int shape = attribute0 >> 14;
            int size  = attribute1 >> 14;
            int prio  = (attribute2 >> 10) & 3;

            width =  g_sprite_size[shape][size][0];
            height = g_sprite_size[shape][size][1];

            if (m_io.vcount >= y && m_io.vcount <= y + height - 1)
            {
                for (int j = 0; j < width; j++)
                {
                    m_buffer[4 + prio][(x + j) % 240] = 0x7FFF;
                }
                /*int line = m_io.vcount - y;

                int number  = attribute2 & 0x3FF;
                int palette = attribute2 >> 12;
                //bool rotate_scale = attribute0 & 256;
                bool h_flip = attribute1 & (1 << 12);
                bool v_flip = attribute1 & (1 << 13);
                bool is_256 = attribute0 & (1 << 13);

                if (is_256) number >>= 1;
                if (v_flip) line = height - line;

                int tile_y     = line & 7;
                int tile_row   = line >> 3;
                int tile_count = width >> 3;

                for (int j = 0; j < tile_count; j++)
                {
                    int _number;

                    if (m_io.control.one_dimensional)
                    {
                        _number = number + tile_row * tile_count + j;
                    }
                    else
                    {
                        _number = number + tile_row * 32 + j;
                    }
                }*/

                offset -= 8;
            }
        }
    }
}
