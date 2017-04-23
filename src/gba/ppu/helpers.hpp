/**
  * Copyright (C) 2017 flerovium^-^ (Frederic Meyer)
  *
  * This file is part of NanoboyAdvance.
  *
  * NanoboyAdvance is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  * 
  * NanoboyAdvance is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  * GNU General Public License for more details.
  * 
  * You should have received a copy of the GNU General Public License
  * along with NanoboyAdvance. If not, see <http://www.gnu.org/licenses/>.
  */

#ifdef PPU_INCLUDE

// TODO: greenswap?
inline u32 color_convert(u16 color) {
    return m_color_lut[color & 0x7FFF];
}

inline u16 read_palette(int palette, int index) {
    
    return (m_pal[(palette << 5) + (index << 1) | 1] << 8) |
            m_pal[(palette << 5) + (index << 1)];
}

inline u16 get_tile_pixel_4bpp(u32 base, int palette, int number, int x, int y) {
    
    u32 offset = base + (number << 5) + (y << 2) + (x >> 1);

    int tuple = m_vram[offset];
    int index = (x & 1) ? (tuple >> 4) : (tuple & 0xF);

    if (index == 0) {
        return COLOR_TRANSPARENT;
    }

    return read_palette(palette, index);
}

inline u16 get_tile_pixel_8bpp(u32 base, int palette, int number, int x, int y) {
    
    u32 offset = base + (number << 6) + (y << 3) + x;
    int index  = m_vram[offset];

    if (index == 0) {
        return COLOR_TRANSPARENT;
    }

    return read_palette(palette, index);
}

inline bool is_visible(int x, const bool inside[3], bool outside) {
    
    auto win_enable = m_io.control.win_enable;
    
    if (win_enable[0] || win_enable[1] || win_enable[2]) {
        
        for (int i = 0; i < 2; i++) {
            if (win_enable[i] && m_win_mask[i][x]) {
                return inside[i];
            }
        }
        
        if (win_enable[2] && m_obj_layer[x].window) {
            return inside[2];
        }
        
        return outside;
    }
    
    return true;
}

static inline float decode_fixed16(u16 number) {
    
    bool  is_negative = number & (1 << 15);
    s32   int_part    = (number >> 8) | (is_negative ? 0xFFFFFF00 : 0);
    float frac_part   = static_cast<float>(number & 0xFF) / 256.0;

    return static_cast<float>(int_part) + frac_part;
}

static inline float decode_fixed32(u32 number) {
    
    bool  is_negative = number & (1 << 27);
    s32   int_part    = ((number & ~0xF0000000) >> 8) | (is_negative ? 0xFFF00000 : 0);
    float frac_part   = static_cast<float>(number & 0xFF) / 256.0;

    return static_cast<float>(int_part) + frac_part;
}

#endif
