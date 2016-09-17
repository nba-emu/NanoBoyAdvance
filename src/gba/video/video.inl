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


#ifndef __NBA_VIDEO_INL__
#define __NBA_VIDEO_INL__


namespace GBA
{
    inline u32 Video::DecodeRGB555(u16 color)
    {
        return 0xFF000000 |
               (((color & 0x1F) * 8) << 16) |
               ((((color >> 5) & 0x1F) * 8) << 8) |
               (((color >> 10) & 0x1F) * 8);
    }

    inline bool Video::IsVisible(int i, bool inside[3], bool outside)
    {
        if (m_Win[0].enable || m_Win[1].enable || m_ObjWin.enable)
        {
            if (m_Win[0].enable && m_WinMask[0][i] == 1)
                return inside[0];
            else if (m_Win[1].enable && m_WinMask[1][i] == 1)
                return inside[1];
            else
                return outside;
        }

        return true;
    }

    inline void Video::DecodeTileLine4BPP(u32 block_base, u32 palette_base, int number, int line)
    {
        u32 offset = block_base + number * 32 + line * 4;
        u8* block_ptr   = m_VRAM + offset;
        u8* palette_ptr = m_PAL + palette_base;

        for (int i = 0; i < 4; ++i)
        {
            u8 value = block_ptr[i];
            int left_index = value & 0xF;
            int right_index = value >> 4;

            if (left_index != 0)
                m_TileBuffer[i * 2] = (palette_ptr[left_index * 2 + 1] << 8) |
                                       palette_ptr[left_index * 2];
            else
                m_TileBuffer[i * 2] = COLOR_TRANSPARENT;

            if (right_index != 0)
                m_TileBuffer[i * 2 + 1] = (palette_ptr[right_index * 2 + 1] << 8) |
                                           palette_ptr[right_index * 2];
            else
                m_TileBuffer[i * 2 + 1] = COLOR_TRANSPARENT;
        }
    }

    inline void Video::DecodeTileLine8BPP(u32 block_base, int number, int line, bool sprite)
    {
        u32 offset = block_base + number * 64 + line * 8;
        u32 palette_base = sprite ? 0x200 : 0x0;

        for (int i = 0; i < 8; i++)
        {
            u8 value = m_VRAM[offset + i];

            if (value == 0)
            {
                m_TileBuffer[i] = COLOR_TRANSPARENT;
                continue;
            }

            m_TileBuffer[i] = (m_PAL[palette_base + value * 2 + 1] << 8) |
                               m_PAL[palette_base + value * 2];
        }
    }

    inline u16 Video::DecodeTilePixel8BPP(u32 block_base, int number, int line, int column, bool sprite)
    {
        u8 value = m_VRAM[block_base + number * 64 + line * 8 + column];
        u32 palette_base = sprite ? 0x200 : 0x0;

        if (value == 0)
            return COLOR_TRANSPARENT;

        return (m_PAL[palette_base + value * 2 + 1] << 8) |
                m_PAL[palette_base + value * 2];
    }

    inline float Video::DecodeGBAFloat16(u16 number)
    {
        bool is_negative = number & (1 << 15);
        s32 int_part = (number >> 8) | (is_negative ? 0xFFFFFF00 : 0);
        float frac_part = static_cast<float>(number & 0xFF) / 256;

        return static_cast<float>(int_part) + frac_part;
    }

    inline float Video::DecodeGBAFloat32(u32 number)
    {
        bool is_negative = number & (1 << 27);
        s32 int_part = ((number & ~0xF0000000) >> 8) | (is_negative ? 0xFFF00000 : 0);
        float frac_part = static_cast<float>(number & 0xFF) / 256;

        return static_cast<float>(int_part) + frac_part;
    }
}


#endif // __NBA_VIDEO_INL__
