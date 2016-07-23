/*
* Copyright (C) 2015 Frederic Meyer
*
* This file is part of nanoboyadvance.
*
* nanoboyadvance is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* nanoboyadvance is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "util/types.h"
#include "util/log.h"
#include "interrupt.h"
#include <cstring>

namespace NanoboyAdvance
{
    class GBAVideo
    {   
        static const int VBLANK_INTERRUPT;
        static const int HBLANK_INTERRUPT;
        static const int VCOUNT_INTERRUPT;

        enum class GBAVideoSpriteShape
        {
            Square = 0,
            Horizontal = 1,
            Vertical = 2,
            Prohibited = 3
        };

        enum class GBASpecialEffect
        {
            None = 0,
            AlphaBlend = 1,
            Increase = 2,
            Decrease = 3
        };
        
        GBAInterrupt* interrupt;
        
        // Rendering buffers
        u32 bd_buffer[240];
        u32 bg_buffer[4][240];
        u32 obj_buffer[4][240];
        
        // Tick counter
        int ticks {0};

        // Decodes GBA RGB555 to ARGB8888 format.
        inline u32 DecodeRGB5(u16 color)
        {
            return 0xFF000000 |
                   (((color & 0x1F) * 8) << 16) |
                   ((((color >> 5) & 0x1F) * 8) << 8) |
                   (((color >> 10) & 0x1F) * 8);
        }

        // Decodes a single 4-bit tile line.
        inline u32* DecodeTileLine4BPP(u32 block_base, u32 palette_base, int number, int line)
        {
            u32 offset = block_base + number * 32 + line * 4;
            u32* data = new u32[8];

            // Empty buffer.
            memset(data, 0, 32);

            for (int i = 0; i < 4; i++)
            {
                u8 value = vram[offset + i];
                int left_index = value & 0xF;
                int right_index = value >> 4;
                u32 left_color = DecodeRGB5((pal[palette_base + left_index * 2 + 1] << 8) | 
                                             pal[palette_base + left_index * 2]);
                u32 right_color = DecodeRGB5((pal[palette_base + right_index * 2 + 1] << 8) | 
                                              pal[palette_base + right_index * 2]);

                // Copy left and right pixel to buffer
                data[i * 2] = (left_index == 0) ? (left_color & ~0xFF000000) : left_color;
                data[i * 2 + 1] = (right_index == 0) ? (right_color & ~0xFF000000) : right_color;
            }

            return data;
        }

        // Decodes a single 8-bit tile line
        inline u32* DecodeTileLine8BPP(u32 block_base, int number, int line, bool sprite)
        {
            u32 offset = block_base + number * 64 + line * 8;
            u32 palette_base = sprite ? 0x200 : 0x0;
            u32* data = new u32[8];

            // Empty buffer.
            memset(data, 0, 32);

            for (int i = 0; i < 8; i++)
            {
                u8 value = vram[offset + i];
                u32 color = DecodeRGB5((pal[palette_base + value * 2 + 1] << 8) | 
                                        pal[palette_base + value * 2]);

                data[i] = (value == 0) ? (color & ~0xFF000000) : color;
            }

            return data;
        }

        // Decodes a single 8-bit tile pixel
        inline u32 DecodeTilePixel8BPP(u32 block_base, int number, int line, int column, bool sprite)
        {
            u8 value = vram[block_base + number * 64 + line * 8 + column];
            u32 palette_base = sprite ? 0x200 : 0x0;
            u32 color = DecodeRGB5((pal[palette_base + value * 2 + 1] << 8) | 
                                    pal[palette_base + value * 2]);
            
            return (value == 0) ? (color & ~0xFF000000) : color;
        }

        // Copies src buffer on dst buffer
        inline void OverlayLineBuffers(u32* dst, u32* src)
        {
            for (int i = 0; i < 240; i++) 
            {
                u32 color = src[i];

                if ((color >> 24) != 0)
                    dst[i] = color | 0xFF000000;
            }
        }

        inline void DrawLineToBuffer(u32* line_buffer, int line, bool backdrop)
        {
            for (int i = 0; i < 240; i++)
            {
                u32 color = line_buffer[i];

                if (backdrop || (color >> 24) != 0)
                    buffer[line * 240 + i] = line_buffer[i] | 0xFF000000;
            }
        }
        
        // Renderers
        void RenderBackgroundMode0(int id, int line);
        void RenderBackgroundMode1(int id, int line);
        void RenderSprites(int priority, int line, u32 tile_base);

        // Renders one entire line
        void Render(int line);
    public:
        enum class GBAVideoState
        {
            Scanline,
            HBlank,
            VBlank
        };

        // Current PPU state
        GBAVideoState state {GBAVideoState::Scanline};

        // Scanline indicator
        bool render_scanline {false};
        
        // PPU memory
        u8 pal[0x400];
        u8 vram[0x18000];
        u8 oam[0x400];
        
        // DMA indicators
        bool hblank_dma {false};
        bool vblank_dma {false};
        
        // Output buffer
        u32 buffer[240 * 160];

        u16 vcount {0};
        int video_mode {0};
        bool frame_select {false};
        bool forced_blank {false};
        bool vcount_flag {false};
        bool vblank_irq {false};
        bool hblank_irq {false};
        bool vcount_irq {false};
        u8 vcount_setting {0};

        struct Background
        {
            bool enable {false};
            bool mosaic {false};
            bool true_color {false};
            bool wraparound {false};
            int priority {0};
            int size {0}; 
            u32 tile_base {0};
            u32 map_base {0};
            u32 x {0};
            u32 y {0};
            u16 x_ref {0};
            u16 y_ref {0};
            u16 x_ref_int {0};
            u16 y_ref_int {0};
            u16 pa {0};
            u16 pb {0};
            u16 pc {0};
            u16 pd {0};
        } bg[4];

        struct Object
        {
            bool enable {false};
            bool hblank_access {false};
            bool two_dimensional {false};
        } obj;

        struct Window
        {
            bool enable {false};
            bool bg_in[4] {false, false, false, false};
            bool obj_in {false};
            bool sfx_in {false};
            u16 left {0};
            u16 right {0};
            u16 top {0};
            u16 bottom {0};
        } win[2];

        struct WindowOuter
        {
            bool bg[4] {false, false, false, false};
            bool obj {false};
            bool sfx {false};
        } winout;

        struct ObjectWindow
        {
            bool enable {false};
            // TODO...
        } objwin;
        
        static inline float DecodeGBAFloat32(u32 number)
        {
            bool is_negative = number & (1 << 27);
            s32 int_part = ((number & ~0xF0000000) >> 8) | (is_negative ? 0xFFF00000 : 0);
            float frac_part = static_cast<float>(number & 0xFF) / 256;

            return static_cast<float>(int_part) + (is_negative ? -frac_part : frac_part);
        }

        static inline float DecodeGBAFloat16(u16 number)
        {
            bool is_negative = number & (1 << 15);
            s32 int_part = (number >> 8) | (is_negative ? 0xFFFFFF00 : 0);
            float frac_part = static_cast<float>(number & 0xFF) / 256;

            return static_cast<float>(int_part) + (is_negative ? -frac_part : frac_part);        
        }

        static inline u32 EncodeGBAFloat32(float number)
        {
            s32 int_part = static_cast<s32>(number);
            u8 frac_part = static_cast<u8>((number - int_part) * (number >= 0 ? 256 : -256)); // optimize

            return (u32)(int_part << 8 | frac_part);
        }

        // Constructor
        GBAVideo(GBAInterrupt* interrupt);
        
        // Update PPU state
        void Step();
    };
}
