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


namespace NanoboyAdvance
{
    void Video::RenderRotateScaleBG(int id)
    {
        u32 tile_base = m_BG[id].tile_base;
        u32 map_base = m_BG[id].map_base;
        bool wraparound = m_BG[id].wraparound;
        float parameter_a = DecodeGBAFloat16(m_BG[id].pa);
        float parameter_c = DecodeGBAFloat16(m_BG[id].pc);
        float x_ref = m_BG[id].x_ref_int;
        float y_ref = m_BG[id].y_ref_int;
        u16* buffer = m_BgBuffer[id];
        int size, block_width;

        // Decode size and block width of the map.
        switch (m_BG[id].size)
        {
        case 0: size = 128; block_width = 16; break;
        case 1: size = 256; block_width = 32; break;
        case 2: size = 512; block_width = 64; break;
        case 3: size = 1024; block_width = 128; break;
        }

        // Draw exactly one line.
        for (int i = 0; i < 240; i++)
        {
            int x = x_ref + i * parameter_a;
            int y = y_ref + i * parameter_c;
            bool is_backdrop = false;

            // Handles x-Coord over-/underflow
            if (x >= size)
            {
                if (wraparound) x = x % size;
                else is_backdrop = true;
            }
            else if (x < 0)
            {
                if (wraparound) x = (size + x) % size;
                else is_backdrop = true;
            }

            // Handles y-Coord over-/underflow
            if (y >= size)
            {
                if (wraparound) y = y % size;
                else is_backdrop = true;
            }
            else if (y < 0)
            {
                if (wraparound) y = (size + y) % size;
                else is_backdrop = true;
            }

            // Handles empty spots.
            if (is_backdrop)
            {
                buffer[i] = COLOR_TRANSPARENT;
                continue;
            }

            // Raster-position of the tile.
            int map_x = x / 8;
            int map_y = y / 8;

            // Position of the wanted pixel inside the tile.
            int tile_x = x % 8;
            int tile_y = y % 8;

            // Get the tile number from the map using the raster-position
            int tile_number = m_VRAM[map_base + map_y * block_width + map_x];

            // Get the wanted pixel.
            buffer[i] = DecodeTilePixel8BPP(tile_base, tile_number, tile_y, tile_x, false);
        }
    }
}
