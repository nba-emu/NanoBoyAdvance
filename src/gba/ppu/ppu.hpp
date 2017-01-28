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
#include "../interrupt.hpp"
#define PPU_INCLUDE

namespace gba
{
    const u16 COLOR_TRANSPARENT = 0x8000;

    class ppu
    {
    private:
        u8* m_pal;
        u8* m_oam;
        u8* m_vram;
        interrupt* m_interrupt = nullptr;

        // rendering buffers
        u8 m_win_mask[2][240];
        u16 m_buffer[5][240];
        u32 m_framebuffer[240*160];

        #include "io.hpp"
        #include "helpers.hpp"

        void render_textmode(int bg);
        void render_bitmap_1();
        void render_bitmap_2();
        void render_bitmap_3();

    public:
        ppu();

        void reset();

        io& get_io()
        {
            return m_io;
        }

        u32* get_framebuffer();
        void set_memory(u8* pal, u8* oam, u8* vram);
        void set_interrupt(interrupt* interrupt);

        void hblank();
        void vblank();
        void scanline();
        void next_line();
    };
}

#undef PPU_INCLUDE
