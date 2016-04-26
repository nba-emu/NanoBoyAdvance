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

#include <cstring>
#include "common/types.h"
#include "common/log.h"
#include "gba_io.h"

using namespace std;

namespace NanoboyAdvance
{
    class GBAVideo
    {
        // Enumerating all video states
        enum class GBAVideoState
        {
            Scanline,
            HBlank,
            VBlank
        };
        
        // Enumerating the possible sprite shapes
        enum class GBAVideoSpriteShape
        {
            Square = 0,
            Horizontal = 1,
            Vertical = 2,
            Prohibited = 3
        };
        
        // IO-interface
        GBAIO* gba_io;
        
        // Current PPU state
        GBAVideoState state {GBAVideoState::Scanline};
        
        // BG-io mappings
        u16* bgcnt[4];
        u16* bghofs[4];
        u16* bgvofs[4];
        u16* bgpa[4];
        u16* bgpb[4];
        u16* bgpc[4];
        u16* bgpd[4];

        // WIN-io mappings
        u16* winv[2];
        u16* winh[2];
        
        // Buffers the various layers
        u32 bg_buffer[4][240];
        u32 obj_buffer[4][240];
        
        // Internal tick-counter
        int ticks {0};
        
        /* !! GBA-float encoders / decoders !! */
        
        inline float DecodeGBAFloat32(u32 number)
        {
            bool is_negative = number & (1 << 27);
            s32 int_part = ((number & ~0xF0000000) >> 8) | (is_negative ? 0xFFF00000 : 0);
            float frac_part = static_cast<float>(number & 0xFF) / 256;
            return static_cast<float>(int_part) + (is_negative ? -frac_part : frac_part);
        }

        inline float DecodeGBAFloat16(u16 number)
        {
            bool is_negative = number & (1 << 15);
            s32 int_part = (number >> 8) | (is_negative ? 0xFFFFFF00 : 0);
            float frac_part = static_cast<float>(number & 0xFF) / 256;
            return static_cast<float>(int_part) + (is_negative ? -frac_part : frac_part);        
        }

        inline u32 EncodeGBAFloat32(float number)
        {
            s32 int_part = static_cast<s32>(number);
            u8 frac_part = static_cast<u8>((number - int_part) * (number >= 0 ? 256 : -256)); // optimize
            return (u32)(int_part << 8 | frac_part);
        }
        
        /* !! Methods for color and tile decoding !! */
        
        inline u32 DecodeRGB5(u16 color)
        {
            u32 argb = 0xFF000000 |
                       (((color & 0x1F) * 8) << 16) |
                       ((((color >> 5) & 0x1F) * 8) << 8) |
                       (((color >> 10) & 0x1F) * 8);
            return argb;
        }

        inline u32* DecodeTileLine4BPP(u32 block_base, u32 palette_base, int number, int line)
        {
            u32 offset = block_base + number * 32 + line * 4;
            u32* data = new u32[8];

            // We don't want to have random data in the buffer
            memset(data, 0, 32);

            for (int i = 0; i < 4; i++)
            {
                u8 value = vram[offset + i];
                int left_index = value & 0xF;
                int right_index = value >> 4;
                u32 left_color = DecodeRGB5((pal[palette_base + left_index * 2 + 1] << 8) | pal[palette_base + left_index * 2]);
                u32 right_color = DecodeRGB5((pal[palette_base + right_index * 2 + 1] << 8) | pal[palette_base + right_index * 2]);

                // Copy left and right pixel to buffer
                data[i * 2] = (left_index == 0) ? (left_color & ~0xFF000000) : left_color;
                data[i * 2 + 1] = (right_index == 0) ? (right_color & ~0xFF000000) : right_color;
            }

            return data;
        }

        inline u32* DecodeTileLine8BPP(u32 block_base, int number, int line, bool sprite)
        {
            u32 offset = block_base + number * 64 + line * 8;
            u32 palette_base = sprite ? 0x200 : 0x0;
            u32* data = new u32[8];

            // We don't want to have random data in the buffer
            memset(data, 0, 32);

            for (int i = 0; i < 8; i++)
            {
                u8 value = vram[offset + i];
                u32 color = DecodeRGB5((pal[palette_base + value * 2 + 1] << 8) | pal[palette_base + value * 2]);
                data[i] = (value == 0) ? (color & ~0xFF000000) : color;
            }

            return data;
        }

        inline u32 DecodeTilePixel8BPP(u32 block_base, int number, int line, int column, bool sprite)
        {
            u8 value = vram[block_base + number * 64 + line * 8 + column];
            u32 palette_base = sprite ? 0x200 : 0x0;
            u32 color = DecodeRGB5((pal[palette_base + value * 2 + 1] << 8) | pal[palette_base + value * 2]);
            return (value == 0) ? (color & ~0xFF000000) : color;
        }
        
        // Renderers
        void RenderBackgroundMode0(int id, int line);
        void RenderBackgroundMode1(int id, int line);
        void RenderSprites(int priority, int line, u32 tile_base);
        
        // Misc
        inline void DrawLineToBuffer(u32* line_buffer, int line, bool backdrop)
        {
            for (int i = 0; i < 240; i++) {
                u32 color = line_buffer[i];
                if (backdrop || (color >> 24) != 0) {
                    // TODO: we shouldn't set MSB to 0xFF if it's already set to that value
                    buffer[line * 240 + i] = line_buffer[i] | 0xFF000000;
                }
            }
        }
        
        // Renders one entire line
		void Render(int line);
    public:
        // Scanline indicator
        bool render_scanline {false};
        
        // PPU memory
        u8 pal[0x400];
        u8 vram[0x18000];
        u8 obj[0x400];
        
        // Internal bgN[x/y] copies
        u32 bgx_int[4];
        u32 bgy_int[4];
        
        // DMA indicators
        bool hblank_dma {false};
        bool vblank_dma {false};
        
        // Output buffer
        u32 buffer[240 * 160];
        
        // Constructor
        GBAVideo(GBAIO* gba_io);
        
        // Runs the PPU for exactly one tick
        void Step();
    };
}
