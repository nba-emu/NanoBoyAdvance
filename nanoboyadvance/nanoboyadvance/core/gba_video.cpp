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

#include "gba_video.h"

namespace NanoboyAdvance
{
    GBAVideo::GBAVideo(GBAIO* gba_io)
    {
        this->gba_io = gba_io;
        state = GBAVideoState::Scanline;
        ticks = 0;
        render_scanline = false;
        memset(pal, 0, 0x400);
        memset(vram, 0, 0x18000);
        memset(obj, 0, 0x400);
        memset(buffer, 0, 240 * 160 * 4);
    }

    inline uint GBAVideo::DecodeRGB5(ushort color)
    {
        uint argb = 0xFF000000 |
                    (((color & 0x1F) * 8) << 16) |
                    ((((color >> 5) & 0x1F) * 8) << 8) |
                    (((color >> 10) & 0x1F) * 8);
        return argb;
    }

    inline uint* GBAVideo::DecodeTileLine4BPP(uint block_base, uint palette_base, int number, int line)
    {
        uint offset = block_base + number * 32 + line * 4;
        uint data[8];

        for (int i = 0; i < 4; i++)
        {
            ubyte value = vram[offset + i];
            int left_index = value & 0xF;
            int right_index = value >> 4;
            data[i * 2] = DecodeRGB5((pal[palette_base + left_index * 2 + 1] << 8) | pal[palette_base + left_index * 2]);
            data[i * 2 + 1] = DecodeRGB5((pal[palette_base + right_index * 2 + 1] << 8) | pal[palette_base + right_index * 2]);
        }

        return data;
    }

    inline uint* GBAVideo::DecodeTileLine8PP(uint block_base, int number, int line, bool sprite)
    {
        uint offset = block_base + number * 64 + line * 8;
        uint palette_base = sprite ? 0x200 : 0x0;
        uint data[8];

        for (int i = 0; i < 8; i++)
        {
            ubyte value = vram[offset + i];
            data[i] = DecodeRGB5((pal[palette_base + value * 2 + 1] << 8) | pal[palette_base + value * 2]);
        }

        return data;
    }

    inline uint* GBAVideo::RenderBackgroundMode0(ushort bg_control, int line, int scroll_x, int scroll_y)
    {
        uint tile_block_base = ((bg_control >> 2) & 3) * 0x4000;
        uint map_block_base = ((bg_control >> 8) & 0x1F) * 0x800;
        bool color_mode = bg_control & (1 << 7) ? true : false; // true = 256, false = 16 colors
        int width;
        int height;
        int wrap_y;
        int tile_internal_y;
        int row;
        uint* line_full;
        uint offset;

        // Decode screen size
        switch (bg_control >> 14)
        {
        case 0: width = 256; height = 256; break;
        case 1: width = 512; height = 256; break;
        case 2: width = 256; height = 512; break;
        case 3: width = 512; height = 512; break;
        }

        // We need to calculate which row (in tiles) will be renderend and which specific line of it
        wrap_y = (line + scroll_y) % height;
        tile_internal_y = wrap_y % 8;
        row = (wrap_y - tile_internal_y) / 8;
        offset = map_block_base + (width / 4) * row; // div 4 because one tile occupies two bytes

        // We will render the entire first line and the copy the visible area from it
        line_full = new uint[width];

        // TODO: HFlip and VFlip
        for (int x = 0; x < width / 8; x++)
        {
            ushort value = (vram[offset + 1] << 8) | vram[offset];
            int number = value & 0x3FF;
            uint* tile_data;

            // Depending on the color resolution the tiles are decoded different..
            if (color_mode)
            {
                tile_data = DecodeTileLine8PP(tile_block_base, number, tile_internal_y, false);
            }
            else
            {
                int palette = value >> 12;
                tile_data = DecodeTileLine4BPP(tile_block_base, palette * 0x20, number, tile_internal_y);
            }

            // Copy rendered tile line to background line buffer
            // TODO: memcpy
            for (int i = 0; i < 8; i++)
            {
                line_full[x * 8 + i] = tile_data[i];
            }
            offset += 2;
        }

        // TODO: Create a new array only containing the visible area with wraparound and scroll x

        return line_full;
    }

    void GBAVideo::Render(int line)
    {
        int mode = gba_io->dispcnt & 7;
        bool bg0_enable = gba_io->dispcnt & (1 << 8) ? true : false;
        bool bg1_enable = gba_io->dispcnt & (1 << 9) ? true : false;
        bool bg2_enable = gba_io->dispcnt & (1 << 10) ? true : false;
        bool bg3_enable = gba_io->dispcnt & (1 << 11) ? true : false;

        ASSERT(mode > 5, LOG_ERROR, "Invalid video mode %d: cannot render", mode);

        // Call mode specific rendering logic
        switch (mode)
        {
        case 0:
        {
            // BG Mode 0 Tile/Map based Text mode
            //if (bg0_enable)
            {
                uint* bg_line = RenderBackgroundMode0(gba_io->bg0cnt, line, gba_io->bg0hofs, gba_io->bg0vofs);
                for (int i = 0; i < 240; i++)
                {
                    buffer[line * 240 + i] = bg_line[i];
                }
            }
            break;
        }
        case 3:
            // BG Mode 3 - 240x160 pixels, 32768 colors
            // Bitmap modes are rendered on BG2 which means we must check if it is enabled
            if (bg2_enable)
            {
                uint offset = line * 240 * 2;
                for (int x = 0; x < 240; x++)
                {
                    buffer[line * 240 + x] = DecodeRGB5((vram[offset + 1] << 8) | vram[offset]);
                    offset += 2;
                }
            }
            break;
        case 4:
            // BG Mode 4 - 240x160 pixels, 256 colors (out of 32768 colors)
            // Bitmap modes are rendered on BG2 which means we must check if it is enabled
            if (bg2_enable)
            {
                uint page = gba_io->dispcnt & 0x10 ? 0xA000 : 0;
                for (int x = 0; x < 240; x++)
                {
                    ubyte index = vram[page + line * 240 + x];
                    ushort rgb5 = pal[index * 2] | (pal[index * 2 + 1] << 8);
                    buffer[line * 240 + x] = DecodeRGB5(rgb5);
                }
            }
            break;
        case 5:
            // BG Mode 5 - 160x128 pixels, 32768 colors
            // Bitmap modes are rendered on BG2 which means we must check if it is enabled
            if (bg2_enable)
            {
                uint offset = (gba_io->dispcnt & 0x10 ? 0xA000 : 0) + line * 160 * 2;
                for (int x = 0; x < 240; x++)
                {
                    if (x < 160 && line < 128)
                    {
                        buffer[line * 240 + x] = DecodeRGB5((vram[offset + 1] << 8) | vram[offset]);
                        offset += 2;
                    }
                    else
                    {
                        // The unused space is filled with the first color from pal ram as far as I can see
                        ushort rgb5 = pal[0] | (pal[1] << 8);
                        buffer[line * 240 + x] = DecodeRGB5(rgb5);
                    }
                }
            }
            break;
        }
    }

    void NanoboyAdvance::GBAVideo::Step()
    {
        int lyc = gba_io->dispstat >> 8;
        bool vcounter_irq_enable = (gba_io->dispstat & (1 << 5)) == (1 << 5);

        // Update tickcount
        ticks++;
        
        // Reset flags
        render_scanline = false;

        // Handle V-Count Setting (LYC)
        gba_io->dispstat &= ~(1 << 2);
        gba_io->dispstat |= gba_io->vcount == lyc ? (1 << 2) : 0;

        switch (state)
        {
        case GBAVideoState::Scanline:
        {
            if (ticks >= 960)
            {
                bool hblank_irq_enable = (gba_io->dispstat & (1 << 4)) == (1 << 4);
                gba_io->dispstat = (gba_io->dispstat & ~3) | 2; // set hblank bit
                state = GBAVideoState::HBlank;
                if (hblank_irq_enable)
                {
                    gba_io->if_ |= 2;
                }
                // Render the current scanline only
                Render(gba_io->vcount);
                // Notify that the screen must be updated
                render_scanline = true;
                ticks = 0;
            }
            break;
        }
        case GBAVideoState::HBlank:
            if (ticks >= 272)
            {
                gba_io->dispstat = gba_io->dispstat & ~2; // clear hblank bit
                gba_io->vcount++;
                if (gba_io->vcount == lyc && vcounter_irq_enable)
                {
                    gba_io->if_ |= 4;
                }
                if (gba_io->vcount == 160)
                {
                    gba_io->dispstat = (gba_io->dispstat & ~3) | 1; // set vblank bit
                    state = GBAVideoState::VBlank;
                }
                else
                {
                    state = GBAVideoState::Scanline;
                }
                ticks = 0;
            }
            break;
        case GBAVideoState::VBlank:
        {
            bool vblank_irq_enable = (gba_io->dispstat & (1 << 3)) == (1 << 3);
            if (ticks >= 1232)
            {
                gba_io->vcount++;
                if (gba_io->vcount == lyc && vcounter_irq_enable)
                {
                    gba_io->if_ |= 4;
                }
                if (vblank_irq_enable && gba_io->vcount == 161)
                {
                    gba_io->if_ |= 1;
                }
                if (gba_io->vcount >= 227) // check wether this must be 227 or 228
                {
                    state = GBAVideoState::Scanline;
                    gba_io->dispstat = gba_io->dispstat & ~3; // clear vblank and hblank bit
                    gba_io->vcount = 0;
                }
                ticks = 0;
            }
            break;
        }
        }
    }
}
