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
        // Assign our IO interface to the object
        this->gba_io = gba_io;

        // Zero init memory buffers
        memset(pal, 0, 0x400);
        memset(vram, 0, 0x18000);
        memset(obj, 0, 0x400);
        memset(buffer, 0, 240 * 160 * 4);
    }

    inline u32 GBAVideo::DecodeRGB5(u16 color)
    {
        u32 argb = 0xFF000000 |
                    (((color & 0x1F) * 8) << 16) |
                    ((((color >> 5) & 0x1F) * 8) << 8) |
                    (((color >> 10) & 0x1F) * 8);
        return argb;
    }

    inline u32* GBAVideo::DecodeTileLine4BPP(u32 block_base, u32 palette_base, int number, int line, bool transparent)
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

            if (left_index != 0 || !transparent)
            {
                data[i * 2] = DecodeRGB5((pal[palette_base + left_index * 2 + 1] << 8) | pal[palette_base + left_index * 2]);
            }

            if (right_index != 0 || !transparent)
            {
                data[i * 2 + 1] = DecodeRGB5((pal[palette_base + right_index * 2 + 1] << 8) | pal[palette_base + right_index * 2]);
            }
        }

        return data;
    }

    inline u32* GBAVideo::DecodeTileLine8PP(u32 block_base, int number, int line, bool sprite, bool transparent)
    {
        u32 offset = block_base + number * 64 + line * 8;
        u32 palette_base = sprite ? 0x200 : 0x0;
        u32* data = new u32[8];

        // We don't want to have random data in the buffer
        memset(data, 0, 32);

        for (int i = 0; i < 8; i++)
        {
            u8 value = vram[offset + i];
            if (value != 0 || !transparent)
            {
                data[i] = DecodeRGB5((pal[palette_base + value * 2 + 1] << 8) | pal[palette_base + value * 2]);
            }
        }

        return data;
    }

    inline u32* GBAVideo::RenderBackgroundMode0(u16 bg_control, int line, int scroll_x, int scroll_y, bool transparent)
    {
        // Base addresses for character *and* map data
        u32 tile_block_base = ((bg_control >> 2) & 3) * 0x4000;
        u32 map_block_base = ((bg_control >> 8) & 0x1F) * 0x800;

        // Mode 0 support both 16 and 256 color
        // true = 256 colors, false = 16 colors
        bool color_mode = bg_control & (1 << 7) ? true : false;

        // A value encoding the bounds of the background
        int screen_size = bg_control >> 14;

        // The actual width and height of the background
        int width;
        int height;

        // The actual line to be rendered with scrolling and wrapround factored
        int wrap_y;

        // The concrete line of the tiles to be rendered
        int tile_internal_y;

        // The current row measured in tiles
        int row;

        // Rendering buffers
        u32* line_full;
        u32* line_visible = new u32[240];

        // Contains the offset of the current tile
        u32 offset;

        // There are two areas which possibly get rendered (the left is always renderend, the right only if the width is 512)
        int left_area = 0;
        int right_area = 0;

        // Decode screen size
        switch (screen_size)
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

        // We will render the entire first line and the copy the visible area from it
        line_full = new u32[width];

        // Determine the areas (left and right) to render (this is a little bit hacky nevertheless it works)
        if (row >= 32)
        {
            switch (screen_size)
            {
            case 2: left_area = 1; right_area = 0; break;
            case 3: left_area = 2; right_area = 3; break;
            }
            row -= 32;
        }
        else
        {
            left_area = 0;
            right_area = 1;
        }

        // Calculate the start address
        offset = map_block_base + left_area * 0x800 + 64 * row;

        for (int x = 0; x < width / 8; x++)
        {
            u16 value = (vram[offset + 1] << 8) | vram[offset];
            int number = value & 0x3FF;
            bool horizontal_flip = (value & (1 << 10)) ? true : false;
            bool vertical_flip = (value & (1 << 11)) ? true : false;
            int this_tile_y = tile_internal_y;            
            u32* tile_data;

            // Apply vertical flip
            if (vertical_flip)
            {
                this_tile_y = 7 - this_tile_y;
            }

            // Depending on the color resolution the tiles are decoded different..
            if (color_mode)
            {
                tile_data = DecodeTileLine8PP(tile_block_base, number, this_tile_y, false, transparent);
            }
            else
            {
                int palette = value >> 12;
                tile_data = DecodeTileLine4BPP(tile_block_base, palette * 0x20, number, this_tile_y, transparent);
            }

            // Copy rendered tile line to background line buffer
            // We must take care of horizontal flip
            for (int i = 0; i < 8; i++)
            {
                if (horizontal_flip)
                {
                    line_full[x * 8 + i] = tile_data[7 - i];
                }
                else
                {
                    line_full[x * 8 + i] = tile_data[i];
                }
            }

            // We don't need this anymore
            delete[] tile_data;

            // Have we just finished the left area?
            if (x == 31)
            {
                // If so we must recalculate the offset to point to the first tile of the right area
                offset = map_block_base + right_area * 0x800 + 64 * row;
            }
            else
            {
                // If not just update the offset as usual
                offset += 2;
            }
        }

        // Copy the visible area with wraparound
        for (int i = 0; i < 240; i++)
        {
            line_visible[i] = line_full[(scroll_x + i) % width];
        }

        // If we don't delete this now, it'll never get deleted
        delete[] line_full;

        return line_visible;
    }

    inline u32* GBAVideo::RenderSprites(u32 tile_base, int line, int priority)
    {
        u32* line_buffer = new u32[240];
        u32 offset = 0;
        bool one_dimensional = gba_io->dispcnt & (1 << 6) ? true : false;

        // We dont want garbage data in the buffer
        memset(line_buffer, 0, 240 * 4);

        // Walk all entries
        for (int i = 0; i < 128; i++)
        {
            u16 attribute0 = (obj[offset + 1] << 8) | obj[offset];
            u16 attribute1 = (obj[offset + 3] << 8) | obj[offset + 2];
            u16 attribute2 = (obj[offset + 5] << 8) | obj[offset + 4];

            // Only render those which have matching priority
            if (((attribute2 >> 10) & 3) == priority)
            {
                int width;
                int height;
                int x = attribute1 & 0x1FF;
                int y = attribute0 & 0xFF;
                GBAVideoSpriteShape shape = static_cast<GBAVideoSpriteShape>(attribute0 >> 14);
                int size = attribute1 >> 14;

                // Decode width and height
                switch (shape)
                {
                case GBAVideoSpriteShape::Square:
                    switch (size)
                    {
                    case 0: width = 8; height = 8; break;
                    case 1: width = 16; height = 16; break;
                    case 2: width = 32; height = 32; break;
                    case 3: width = 64; height = 64; break;
                    }
                    break;
                case GBAVideoSpriteShape::Horizontal:
                    switch (size)
                    {
                    case 0: width = 16; height = 8; break;
                    case 1: width = 32; height = 8; break;
                    case 2: width = 32; height = 16; break;
                    case 3: width = 64; height = 32; break;
                    }
                    break;
                case GBAVideoSpriteShape::Vertical:
                    switch (size)
                    {
                    case 0: width = 8; height = 16; break;
                    case 1: width = 8; height = 32; break;
                    case 2: width = 16; height = 32; break;
                    case 3: width = 32; height = 64; break;
                    }
                    break;
                case GBAVideoSpriteShape::Prohibited:
                    width = 0;
                    height = 0;
                    break;
                }

                // Determine if there is something to render for this sprite
                if (line >= y && line <= y + height)
                {
                    int internal_line = line - y;
                    int displacement_y;// = internal_line % 8;
                    int row;// = (internal_line - displacement_y) / 8;//(line - y) / 8;
                    int tiles_per_row = width / 8;
                    int tile_number = (attribute2 & 0x3FF) * 2;
                    int palette_number = attribute2 >> 12;
                    bool rotate_scale = attribute0 & (1 << 8) ? true : false;
                    bool horizontal_flip = !rotate_scale && (attribute1 & (1 << 12));
                    bool vertical_flip = !rotate_scale && (attribute1 & (1 << 13));
                    bool color_mode = attribute0 & (1 << 13) ? true : false;

                    // Apply (outer) vertical flip if required
                    if (vertical_flip) 
                    {
                        internal_line = height - internal_line;                    
                    }

                    // Calculate some stuff
                    displacement_y = internal_line % 8;
                    row = (internal_line - displacement_y) / 8;

                    // Apply (inner) vertical flip
                    if (vertical_flip)
                    {
                        displacement_y = 7 - displacement_y;
                        row = (height / 8) - row;
                    }

                    // Render all visible tiles of the sprite
                    for (int j = 0; j < tiles_per_row; j++)
                    {
                        int current_tile_number;
                        u32* tile_data;

                        // Determine the tile to render
                        if (one_dimensional)
                        {
                            current_tile_number = tile_number + row * tiles_per_row + j * (color_mode ? 2 : 1);
                        }
                        else
                        {
                            current_tile_number = tile_number + row * 32 + j * (color_mode ? 2 : 1);
                        }

                        // Render either in 256 colors or 16 colors mode
                        if (color_mode)
                        {
                            // 256 colors
                            tile_data = DecodeTileLine8PP(tile_base, current_tile_number, displacement_y, true, true);
                        }
                        else
                        {
                            // 16 colors (use palette_nummer)
                            tile_data = DecodeTileLine4BPP(tile_base, 0x200 + palette_number * 0x20, current_tile_number, displacement_y, true);
                        }

                        // Copy data
                        for (int k = 0; k < 8; k++)
                        {
                            if (tile_data[k] != 0 && (x + j * 8 + k) < 240)
                            {
                                line_buffer[x + j * 8 + k] = tile_data[k];
                            }
                        }

                        // We don't need that memory anymore
                        delete[] tile_data;
                    }
                }
            }

            // Update offset to current entry
            offset += 8;
        }

        return line_buffer;
    }

    inline void GBAVideo::DrawLineToBuffer(u32* line_buffer, int line)
    {
        for (int i = 0; i < 240; i++)
        {
            if (line_buffer[i] != 0)
            {
                buffer[line * 240 + i] = line_buffer[i];
            }
        }
    }

    void GBAVideo::Render(int line)
    {
        int mode = gba_io->dispcnt & 7;
        bool bg0_enable = gba_io->dispcnt & (1 << 8) ? true : false;
        bool bg1_enable = gba_io->dispcnt & (1 << 9) ? true : false;
        bool bg2_enable = gba_io->dispcnt & (1 << 10) ? true : false;
        bool bg3_enable = gba_io->dispcnt & (1 << 11) ? true : false;
        bool obj_enable = gba_io->dispcnt & (1 << 12) ? true : false;

        ASSERT(mode > 5, LOG_ERROR, "Invalid video mode %d: cannot render", mode);

        // Emulate the effect caused by "Forced Blank"
        if (gba_io->dispcnt & (1 << 7))
        {
            for (int i = 0; i < 240; i++)
            {
                buffer[line * 240 + i] = 0xFFF8F8F8;
            }
            return;
        }

        // Call mode specific rendering logic
        switch (mode)
        {
        case 0:
        case 1: // just for the sake of it (lol)
        {
            bool first_background = true;
            int bg0_priority = gba_io->bg0cnt & 3;
            int bg1_priority = gba_io->bg1cnt & 3;
            int bg2_priority = gba_io->bg2cnt & 3;
            int bg3_priority = gba_io->bg3cnt & 3;

            for (int i = 3; i >= 0; i--)
            {
                if (bg3_enable && bg3_priority == i)
                {
                    u32* bg_buffer = RenderBackgroundMode0(gba_io->bg3cnt, line, gba_io->bg3hofs, gba_io->bg3vofs, !first_background);
                    DrawLineToBuffer(bg_buffer, line);
                    first_background = false;
                    delete[] bg_buffer;
                }
                else if (bg2_enable && bg2_priority == i)
                {
                    u32* bg_buffer = RenderBackgroundMode0(gba_io->bg2cnt, line, gba_io->bg2hofs, gba_io->bg2vofs, !first_background);
                    DrawLineToBuffer(bg_buffer, line);
                    first_background = false;
                    delete[] bg_buffer;
                }
                else if (bg1_enable && bg1_priority == i)
                {
                    u32* bg_buffer = RenderBackgroundMode0(gba_io->bg1cnt, line, gba_io->bg1hofs, gba_io->bg1vofs, !first_background);
                    DrawLineToBuffer(bg_buffer, line);
                    first_background = false;
                    delete[] bg_buffer;
                }
                else if (bg0_enable && bg0_priority == i)
                {
                    u32* bg_buffer = RenderBackgroundMode0(gba_io->bg0cnt, line, gba_io->bg0hofs, gba_io->bg0vofs, !first_background);
                    DrawLineToBuffer(bg_buffer, line);
                    first_background = false;
                    delete[] bg_buffer;
                }
                if (obj_enable)
                {
                    u32* obj_buffer = RenderSprites(0x10000, line, i);
                    DrawLineToBuffer(obj_buffer, line);
                    delete[] obj_buffer;
                }
            }
            break;
        }
        case 3:
            // BG Mode 3 - 240x160 pixels, 32768 colors
            // Bitmap modes are rendered on BG2 which means we must check if it is enabled
            if (bg2_enable)
            {
                u32 offset = line * 240 * 2;
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
                u32 page = gba_io->dispcnt & 0x10 ? 0xA000 : 0;
                for (int x = 0; x < 240; x++)
                {
                    u8 index = vram[page + line * 240 + x];
                    u16 rgb5 = pal[index * 2] | (pal[index * 2 + 1] << 8);
                    buffer[line * 240 + x] = DecodeRGB5(rgb5);
                }
            }
            break;
        case 5:
            // BG Mode 5 - 160x128 pixels, 32768 colors
            // Bitmap modes are rendered on BG2 which means we must check if it is enabled
            if (bg2_enable)
            {
                u32 offset = (gba_io->dispcnt & 0x10 ? 0xA000 : 0) + line * 160 * 2;
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
                        u16 rgb5 = pal[0] | (pal[1] << 8);
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
                //if (!render_disable)
                {
                    Render(gba_io->vcount);
                    render_scanline = true;
                }
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
