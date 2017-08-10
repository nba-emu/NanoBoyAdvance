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
inline u32 rgb555ToARGB(u16 color) {
    return m_color_lut[color & 0x7FFF];
}

inline u16 readPaletteEntry(int palette, int index) {
    return (m_pal[(palette << 5) + (index << 1) | 1] << 8) |
            m_pal[(palette << 5) + (index << 1)];
}

inline u16 getTilePixel4BPP(u32 base, int palette, int number, int x, int y) {
    u32 offset = base + (number << 5) + (y << 2) + (x >> 1);

    int tuple = m_vram[offset];
    int index = (x & 1) ? (tuple >> 4) : (tuple & 0xF);

    if (index == 0) {
        return COLOR_TRANSPARENT;
    }

    return readPaletteEntry(palette, index);
}

inline u16 getTilePixel8BPP(u32 base, int palette, int number, int x, int y) {
    u32 offset = base + (number << 6) + (y << 3) + x;
    int index  = m_vram[offset];

    if (index == 0) {
        return COLOR_TRANSPARENT;
    }

    return readPaletteEntry(palette, index);
}

inline void drawTileLine4BPP(u32* buffer, u32 base, int palette, int number, int y, bool flip) {
    u8* data = &m_vram[base + (number << 5) + (y << 2)];
    
    if (flip) {
        for (int i = 0; i < 4; i++) {
            int tuple  = *data++;
            int pixel1 = tuple & 15;
            int pixel2 = tuple >> 4;
            
            buffer[((i<<1)|0)^7] = (pixel1 == 0) ? COLOR_TRANSPARENT : readPaletteEntry(palette, pixel1);
            buffer[((i<<1)|1)^7] = (pixel2 == 0) ? COLOR_TRANSPARENT : readPaletteEntry(palette, pixel2);
        }
    } else {
        for (int i = 0; i < 4; i++) {
            int tuple  = *data++;
            int pixel1 = tuple & 15;
            int pixel2 = tuple >> 4;
            
            buffer[(i<<1)|0] = (pixel1 == 0) ? COLOR_TRANSPARENT : readPaletteEntry(palette, pixel1);
            buffer[(i<<1)|1] = (pixel2 == 0) ? COLOR_TRANSPARENT : readPaletteEntry(palette, pixel2);
        }
    }
}

inline void drawTileLine8BPP(u32* buffer, u32 base, int number, int y, bool flip) {
    u8* data = &m_vram[base + (number << 6) + (y << 3)];
    
    if (flip) {
        for (int x = 7; x >= 0; x--) {
            int pixel = *data++;
            if (pixel == 0) {
                buffer[x] = COLOR_TRANSPARENT;
                continue;
            }
            buffer[x] = readPaletteEntry(0, pixel);
        }
    } else {
        for (int x = 0; x < 8; x++) {
            int pixel = *data++;
            if (pixel == 0) {
                buffer[x] = COLOR_TRANSPARENT;
                continue;
            }
            buffer[x] = readPaletteEntry(0, pixel);
        }
    }
}

static inline float decodeFixed16(u16 number) {
    bool  is_negative = number & (1 << 15);
    s32   int_part    = (number >> 8) | (is_negative ? 0xFFFFFF00 : 0);
    float frac_part   = static_cast<float>(number & 0xFF) / 256.0;

    return static_cast<float>(int_part) + frac_part;
}

static inline float decodeFixed32(u32 number) {
    bool  is_negative = number & (1 << 27);
    s32   int_part    = ((number & ~0xF0000000) >> 8) | (is_negative ? 0xFFF00000 : 0);
    float frac_part   = static_cast<float>(number & 0xFF) / 256.0;

    return static_cast<float>(int_part) + frac_part;
}

#endif
