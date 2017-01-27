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

#pragma once

#include "util/integer.hpp"
#define PPU_INCLUDE

namespace gba
{
    class ppu
    {
    private:
        u8* m_pal;
        u8* m_oam;
        u8* m_vram;
        u32 m_framebuffer[240*160];

        #include "io.hpp"

    public:
        ppu();

        void reset();

        u32* get_framebuffer();
        void set_memory(u8* pal, u8* oam, u8* vram);

        void hblank();
        void vblank();
        void scanline();
    };
}

#undef PPU_INCLUDE
