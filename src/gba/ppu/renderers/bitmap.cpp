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

namespace GameBoyAdvance {
    
    void PPU::render_bitmap_1()
    {
        u32 offset = m_io.vcount * 480;

        for (int x = 0; x < 240; x++) {
            m_buffer[2][x] = (m_vram[offset + 1] << 8) | m_vram[offset];
            offset += 2;
        }
    }

    void PPU::render_bitmap_2() {
        u32 page = m_io.control.frame_select ? 0xA000 : 0;
        u32 offset = page + m_io.vcount * 240;

        for (int x = 0; x < 240; x++) {
            int index = m_vram[offset + x];
            m_buffer[2][x] = read_palette(0, index);
        }
    }

    void PPU::render_bitmap_3() {
        u32 page = m_io.control.frame_select ? 0xA000 : 0;
        u32 offset = page + m_io.vcount * 320;

        for (int x = 0; x < 240; x++) {
            if (x < 160 && m_io.vcount < 128) {
                m_buffer[2][x] = (m_vram[offset + 1] << 8) | m_vram[offset];
                offset += 2;
            } else {
                m_buffer[2][x] = COLOR_TRANSPARENT;
            }
        }
    }
}
