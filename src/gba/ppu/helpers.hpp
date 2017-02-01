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

#ifdef PPU_INCLUDE

// TODO: greenswap?
inline u32 color_convert(u16 color)
{
    int r = color & 0x1F;
    int g = (color >> 5) & 0x1F;
    int b = (color >> 10) & 0x1F;

    return 0xFF000000 | (b << 3) | (g << 11) | (r << 19);
}

inline void get_tile_line_4bpp(u8* buffer, u32 base, int number, int y)
{
    u32 offset = base + (number<<5) + (y<<2);

    for (int i = 0; i < 4; i++)
    {
        int indices = m_vram[offset + i];
        buffer[i<<1]     = indices & 0xF;
        buffer[(i<<1)|1] = indices>>4;
    }
}

inline void get_tile_line_8bpp(u8* buffer, u32 base, int number, int y)
{
    u32 offset = base + (number<<6) + (y<<3);

    for (int i = 0; i < 8; i++)
    {
        buffer[i] = m_vram[offset + i];
    }
}

#endif
