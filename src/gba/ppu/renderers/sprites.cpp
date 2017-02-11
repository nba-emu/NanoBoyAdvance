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

namespace GameBoyAdvance
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
            s32 x = attribute1 & 0x1FF;
            int y = attribute0 & 0x0FF;
            int shape = attribute0 >> 14;
            int size  = attribute1 >> 14;
            int prio  = (attribute2 >> 10) & 3;

            if (x & 0x100) x |= 0xFFFFFF00;
            if (y & 0x080) y |= 0xFFFFFF00;

            bool rotate_scale = attribute0 & 256;

            /*if (rotate_scale)
            {
                int group = (attribute1 >> 9) & 0x1F;

                u16 parameter_a = (m_oam[(group << 1) + 0x7 ] << 8) | m_oam[(group << 1) + 0x6 ];
                u16 parameter_b = (m_oam[(group << 1) + 0xF ] << 8) | m_oam[(group << 1) + 0xE ];
                u16 parameter_c = (m_oam[(group << 1) + 0x17] << 8) | m_oam[(group << 1) + 0x16];
                u16 parameter_d = (m_oam[(group << 1) + 0x1F] << 8) | m_oam[(group << 1) + 0x1E];

                y = ppu::decode_float16(parameter_c) * x + ppu::decode_float16(parameter_d) * y;
            }*/

            width =  g_sprite_size[shape][size][0];
            height = g_sprite_size[shape][size][1];

            if (m_io.vcount >= y && m_io.vcount <= y + height - 1)
            {
                int line = m_io.vcount - y;

                int number  = attribute2 & 0x3FF;
                int palette = (attribute2 >> 12) + 16;
                bool h_flip = !rotate_scale && (attribute1 & (1 << 12));
                bool v_flip = !rotate_scale && (attribute1 & (1 << 13));
                bool is_256 = attribute0 & (1 << 13);

                if (is_256) number >>= 1;
                if (v_flip) line = height - line;

                int tile_y     = line & 7;
                int tile_row   = line >> 3;
                int tile_count = width >> 3;

                if (v_flip)
                {
                    tile_y ^= 7;
                    tile_row = (height >> 3) - tile_row;
                }

                if (m_io.control.one_dimensional)
                {
                    number += tile_row * tile_count;
                }
                else
                {
                    number += tile_row << 5;
                }

                int pos = 0;

                if (x < 0) pos = x * -1;

                for (; pos < width && (x + pos) < 240; pos++)
                {
                    u16 pixel;
                    int tile_x = pos & 7;
                    int tile   = pos >> 3;

                    if (is_256)
                    {
                        pixel = get_tile_pixel_8bpp(tile_base, 16, number + tile, tile_x, tile_y);
                    }
                    else
                    {
                        pixel = get_tile_pixel_4bpp(tile_base, palette, number + tile, tile_x, tile_y);
                    }

                    if (pixel != COLOR_TRANSPARENT)
                        m_buffer[4 + prio][x + pos] = pixel;
                }
            }

            offset -= 8;
        }
    }
}
