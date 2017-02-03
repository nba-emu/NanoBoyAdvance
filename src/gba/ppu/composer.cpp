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

#include "ppu.hpp"

namespace gba
{
    void ppu::compose_scanline()
    {
        u16 backdrop_color = (m_pal[1] << 8) | m_pal[0];
        u32* line_buffer = m_framebuffer  + m_io.vcount * 240;

        for (int i = 0; i < 240; i++)
        {
            int layer[2] = { 5, 5 };
            u16 pixel[2] = { backdrop_color, 0 };

            for (int j = 3; j >= 0; j--)
            {
                for (int k = 3; k >= 0; k--)
                {
                    u16 new_pixel = m_buffer[k][i];

                    if (new_pixel != COLOR_TRANSPARENT && m_io.control.enable[k] &&
                            m_io.bgcnt[k].priority == j)
                    {
                        layer[1] = layer[0];
                        layer[0] = k;
                        pixel[1] = pixel[0];
                        pixel[0] = new_pixel;
                    }
                }

                if (m_io.control.enable[4])
                {
                    u16 new_pixel = m_buffer[4 + j][i];

                    if (new_pixel != COLOR_TRANSPARENT)
                    {
                        layer[1] = layer[0];
                        layer[0] = 4;
                        pixel[1] = pixel[0];
                        pixel[0] = new_pixel;
                    }
                }
            }

            // SFX code

            line_buffer[i] = color_convert(pixel[0]);
        }
    }
}
