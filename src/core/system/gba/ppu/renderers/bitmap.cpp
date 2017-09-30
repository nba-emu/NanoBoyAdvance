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

#include "../ppu.hpp"

namespace Core {
    
    void PPU::renderBitmapMode1BG() {
        u32 offset = regs.vcount * 480;

        for (int x = 0; x < 240; x++) {
            m_buffer[2][x] = (m_vram[offset + 1] << 8) | m_vram[offset];
            offset += 2;
        }
    }

    void PPU::renderBitmapMode2BG() {
        u32 page = regs.control.frame_select ? 0xA000 : 0;
        u32 offset = page + regs.vcount * 240;

        for (int x = 0; x < 240; x++) {
            int index = m_vram[offset + x];
            m_buffer[2][x] = readPaletteEntry(0, index);
        }
    }

    void PPU::renderBitmapMode3BG() {
        u32 page = regs.control.frame_select ? 0xA000 : 0;
        u32 offset = page + regs.vcount * 320;

        for (int x = 0; x < 240; x++) {
            if (x < 160 && regs.vcount < 128) {
                m_buffer[2][x] = (m_vram[offset + 1] << 8) | m_vram[offset];
                offset += 2;
            } else {
                m_buffer[2][x] = COLOR_TRANSPARENT;
            }
        }
    }
}
