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

inline u16 read_palette(int palette, int index)
{
    return (m_pal[(palette << 5) + (index << 1) | 1] << 8) |
            m_pal[(palette << 5) + (index << 1)];
}

inline u16 get_tile_pixel_4bpp(u32 base, int palette, int number, int x, int y)
{
    u32 offset = base + (number << 5) + (y << 2) + (x >> 1);

    int tuple = m_vram[offset];
    int index = (x & 1) ? (tuple >> 4) : (tuple & 0xF);

    if (index == 0)
    {
        return COLOR_TRANSPARENT;
    }

    return read_palette(palette, index);
}

inline u16 get_tile_pixel_8bpp(u32 base, int palette, int number, int x, int y)
{
    u32 offset = base + (number << 6) + (y << 3) + x;
    int index  = m_vram[offset];

    if (index == 0)
    {
        return COLOR_TRANSPARENT;
    }

    return read_palette(palette, index);
}

static inline float decode_float16(u16 number)
{
    bool  is_negative = number & (1 << 15);
    s32   int_part    = (number >> 8) | (is_negative ? 0xFFFFFF00 : 0);
    float frac_part   = static_cast<float>(number & 0xFF) / 256;

    return static_cast<float>(int_part) + frac_part;
}

static inline float decode_float32(u32 number)
{
    bool  is_negative = number & (1 << 27);
    s32   int_part    = ((number & ~0xF0000000) >> 8) | (is_negative ? 0xFFF00000 : 0);
    float frac_part   = static_cast<float>(number & 0xFF) / 256;

    return static_cast<float>(int_part) + frac_part;
}

#endif
