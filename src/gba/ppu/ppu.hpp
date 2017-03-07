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

#pragma once

#include "util/integer.hpp"
#include "../cpu/interrupt.hpp"
#define PPU_INCLUDE

namespace GameBoyAdvance {
    
    const u16 COLOR_TRANSPARENT = 0x8000;

    class PPU {
    private:
        u8* m_pal;
        u8* m_oam;
        u8* m_vram;
        Interrupt* m_interrupt = nullptr;

        // rendering buffers
        u8 m_win_mask[2][240];
        u16 m_buffer[8][240];
        u32 m_framebuffer[240*160];

        int m_frameskip;
        int m_frame_counter;

        #include "io.hpp"
        #include "helpers.hpp"

        void render_text(int id);
        void render_affine(int id);
        void render_bitmap_1();
        void render_bitmap_2();
        void render_bitmap_3();
        void render_obj(u32 tile_base);

        template <bool is_256, int id>
        void render_text_internal();
        
    public:
        PPU();

        void reset();

        IO& get_io() {
            return m_io;
        }

        u32* get_framebuffer();
        void set_frameskip(int frames);
        void set_memory(u8* pal, u8* oam, u8* vram);
        void set_interrupt(Interrupt* interrupt);

        void hblank();
        void vblank();
        void scanline();
        void next_line();

        void compose_scanline();
    };
}

#undef PPU_INCLUDE
