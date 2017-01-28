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

inline void decode_tile_line_4bpp(u16* buffer, u32 block_base, u32 palette_base, int number, int line)
{
    u32 offset = block_base + (number << 5) + (line << 2);
    u8* block_ptr   = m_vram + offset;
    u8* palette_ptr = m_pal + palette_base;

    for (int i = 0; i < 4; i++)
    {
        int indices[2];
        int encoder = block_ptr[i];

        indices[0] = encoder & 0xF;
        indices[1] = encoder >> 4;

        for (int j = 0; j < 2; j++)
        {
            int index = indices[j] << 1;

            if (index == 0)
            {
                buffer[(i << 1) + j] = COLOR_TRANSPARENT;
                continue;
            }

            buffer[(i << 1) + j] = (palette_ptr[index + 1] << 8) | palette_ptr[index];
        }
    }
}

inline void decode_tile_line_8bpp(u16* buffer, u32 block_base, int number, int line, bool sprite)
{
    u32 offset = block_base + (number << 6) + (line << 3);
    u32 palette_base = sprite ? 0x200 : 0;
    u8* block_ptr   = m_vram + offset;
    u8* palette_ptr = m_pal + palette_base;

    for (int i = 0; i < 8; i++)
    {
        int index = block_ptr[i] << 1;

        if (index == 0)
        {
            buffer[i] = COLOR_TRANSPARENT;
            continue;
        }

        buffer[i] = (palette_ptr[index + 1] << 8) | palette_ptr[index];
    }
}

#endif
