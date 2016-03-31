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
        
        // Internal tick-counter
        int ticks {0};
        
        // GBA-float encoders / decoders
        inline float DecodeGBAFloat32(u32 number);
        inline float DecodeGBAFloat16(u16 number);
        inline u32 EncodeGBAFloat32(float number);
        
        // Methods for color and tile decoding
        inline u32 DecodeRGB5(u16 color);
		inline u32* DecodeTileLine4BPP(u32 block_base, u32 palette_base, int number, int line, bool transparent);
		inline u32* DecodeTileLine8BPP(u32 block_base, int number, int line, bool sprite, bool transparent);
        inline u32 DecodeTilePixel8BPP(u32 block_base, int number, int line, int column, bool sprite, bool transparent);
        
        // Renderers
        u32* RenderBackgroundMode0(int id, int line, bool transparent);
        u32* RenderBackgroundMode1(int id, int line, bool transparent);
        u32* RenderSprites(u32 tile_base, int line, int priority);
        
        // Misc
        inline void DrawLineToBuffer(u32* line_buffer, int line);
        
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
        
        // Output buffer
        u32 buffer[240 * 160];
        
        // Constructor
        GBAVideo(GBAIO* gba_io);
        
        // Runs the PPU for exactly one tick
        void Step();
    };
}