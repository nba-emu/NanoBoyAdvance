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

namespace gba
{
    void ppu::render_textmode(int id)
    {
        u16* buffer = m_buffer[id];
        auto bg = m_io.bgcnt[id];

        // background dimensions
        int width = ((bg.screen_size & 1) + 1) << 8;
        int height = ((bg.screen_size >> 1) + 1) << 8;

        for (int x = 0; x < 240; x++)
        {
            buffer[x] = 0x1F;
        }
    }
}
