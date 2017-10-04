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

#include "enums.hpp"
#include "util/integer.hpp"
#include "../interrupt.hpp"
#include "../config.hpp"

#define PPU_INCLUDE

namespace Core {

    const u16 COLOR_TRANSPARENT = 0x8000;

    class PPU {
    private:
        u8* m_pal;
        u8* m_oam;
        u8* m_vram;
        Interrupt* m_interrupt = nullptr;

        int  m_frameskip;
        u32* m_framebuffer;
        Config* m_config;

        // rendering buffers
        u16  m_buffer[4][240];
        bool m_win_mask[2][240];
        bool m_win_scanline_enable[2];

        // color conversion LUT
        u32 m_color_lut[0x8000];

        // LUT for fast (alpha) blend calculation
        u8 blend_table[17][17][32][32];

        bool line_has_alpha_objs;

        struct ObjectPixel {
            u8  prio;
            u16 pixel;
            bool alpha;
            bool window;
        } m_obj_layer[240];

        int m_frame_counter;

        #include "io.hpp"
        #include "helpers.hpp"

        void renderTextBG(int id);
        void renderAffineBG(int id);
        void renderBitmapMode1BG();
        void renderBitmapMode2BG();
        void renderBitmapMode3BG();
        void renderSprites();

        void blendPixels(u16* target1, u16 target2, SpecialEffect sfx);

    public:
        PPU(Config* config, u8* paletteMemory, u8* objectMemory, u8* videoMemory);

        void reset();
        void reloadConfig();

        IO& getIO() {
            return regs;
        }

        void setMemoryBuffers(u8* pal, u8* oam, u8* vram);
        void setInterruptController(Interrupt* interrupt);

        void hblank();
        void vblank();
        void scanline(bool render);
        void nextLine();

        void renderWindow(int id);

        void completeScanline();
    };
}

#undef PPU_INCLUDE
