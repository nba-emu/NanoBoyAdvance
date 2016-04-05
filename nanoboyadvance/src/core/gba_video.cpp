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

/* TODO: 1) Improve RenderSprites method and allow for OBJWIN
 *          mask generation
 *       2) Implement OBJWIN
 *       3) Lookup if offset to RenderSprites is always constant
 *       4) Fix rotate-scale logic and apply to RenderSprites and Mode3-5
 */

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
        
        // Init bgNcnt mapping
        bgcnt[0] = &gba_io->bg0cnt;
        bgcnt[1] = &gba_io->bg1cnt;
        bgcnt[2] = &gba_io->bg2cnt;
        bgcnt[3] = &gba_io->bg3cnt;
        
        // Init bgN[h/v]ofs mapping
        bghofs[0] = &gba_io->bg0hofs;
        bghofs[1] = &gba_io->bg1hofs;
        bghofs[2] = &gba_io->bg2hofs;
        bghofs[3] = &gba_io->bg3hofs;
        bgvofs[0] = &gba_io->bg0vofs;
        bgvofs[1] = &gba_io->bg1vofs;
        bgvofs[2] = &gba_io->bg2vofs;
        bgvofs[3] = &gba_io->bg3vofs;
        
        // Init bgNp[a/b/c/d] mapping
        bgpa[2] = &gba_io->bg2pa;
        bgpa[3] = &gba_io->bg3pa;
        bgpb[2] = &gba_io->bg2pb;
        bgpb[3] = &gba_io->bg3pb;
        bgpc[2] = &gba_io->bg2pc;
        bgpc[3] = &gba_io->bg3pc;
        bgpd[2] = &gba_io->bg2pd;
        bgpd[3] = &gba_io->bg3pd;
    }

    void GBAVideo::RenderBackgroundMode0(int id, int line)
    {
        // IO-register values
        u16 bg_control = *bgcnt[id];
        u16 scx = *bghofs[id];
        u16 scy = *bgvofs[id];
        
        // Rendering variables
        u32 tile_block_base = ((bg_control >> 2) & 0x03) * 0x4000;
        u32 map_block_base = ((bg_control >> 8) & 0x1F) * 0x800;
        bool color_mode = bg_control & (1 << 7);
        int width = (((bg_control >> 14) & 1) + 1) << 8;
        int height = ((bg_control >> 15) + 1) << 8;
        int map_line = (line + scy) % height;
        int tile_line = map_line % 8;
        int map_row = (map_line - tile_line) / 8;
        u32* line_buffer = new u32[width];
        int left_area;
        int right_area;
        u32 offset;
        
        // Map may consist of 1 to max. 4 screen areas, 1 or 2 will be rendered.
        if (map_row >= 32) {
            left_area = width == 512 ? 2 : 1;
            right_area = 3;
            map_row -= 32;
        } else {
            left_area = 0;
            right_area = 1;
        }
        
        // Calculate the offset of the first tile encoder.
        offset = map_block_base + left_area * 0x800 + 64 * map_row;
        
        // Decode tile information for each tile in the row and render them.
        for (int x = 0; x < width / 8; x++) {
            u16 tile_encoder = (vram[offset + 1] << 8) | vram[offset];
            int tile_number = tile_encoder & 0x3FF;
            bool horizontal_flip = tile_encoder & (1 << 10);
            bool vertical_flip = tile_encoder & (1 << 11);       
            u32* tile_data;
            
            // Background may be either 16 or 256 colors, render the given tile.
            if (color_mode) {
                tile_data = DecodeTileLine8BPP(tile_block_base, tile_number, (vertical_flip ? (7 - tile_line) : tile_line), false);
            } else {
                int palette = tile_encoder >> 12;
                tile_data = DecodeTileLine4BPP(tile_block_base, palette * 0x20, tile_number, (vertical_flip ? (7 - tile_line) : tile_line));
            }
            
            // Copy rendered tile to line buffer.
            for (int i = 0; i < 8; i++) {
                line_buffer[x * 8 + i] = tile_data[horizontal_flip ? (7 - i) : i];
            }
            
            delete[] tile_data;
            
            // Update offset.
            if (x == 31) offset = map_block_base + right_area * 0x800 + 64 * map_row;
            else offset += 2;
        }
        
        // Copy *visible* pixels with wraparound.
        for (int i = 0; i < 240; i++) {
            bg_buffer[id][i] = line_buffer[(scx + i) % width];
        }

        delete[] line_buffer;
    }

    void GBAVideo::RenderBackgroundMode1(int id, int line)
    {
        // IO-register values
        u16 bg_control = *bgcnt[id];
        u32 bgx_internal = bgx_int[id];
        u32 bgy_internal = bgy_int[id];
        u16 bgpa = *this->bgpa[id];
        u16 bgpb = *this->bgpb[id];
        u16 bgpc = *this->bgpc[id];
        u16 bgpd = *this->bgpd[id];
        
        // Rendering variables
        u32 tile_block_base = ((bg_control >> 2) & 0x03) * 0x4000;
        u32 map_block_base = ((bg_control >> 8) & 0x1F) * 0x800;
        bool wraparound = bg_control & (1 << 13);
        int blocks = ((bg_control >> 14) + 1) << 4;
        int size = blocks * 8;
        
        for (int i = 0; i < 240; i++) {
            float dec_bgx = DecodeGBAFloat32(bgx_internal);
            float dec_bgy = DecodeGBAFloat32(bgy_internal);
            float dec_bgpa = DecodeGBAFloat16(bgpa);
            float dec_bgpb = DecodeGBAFloat16(bgpb);
            float dec_bgpc = DecodeGBAFloat16(bgpc);
            float dec_bgpd = DecodeGBAFloat16(bgpd);
            int x = dec_bgx + (dec_bgpa * i) + (dec_bgpb * line);
            int y = dec_bgy + (dec_bgpc * i) + (dec_bgpd * line);
            int tile_internal_line;
            int tile_internal_x;
            int tile_row;
            int tile_column;
            int tile_number;
            
            if ((x >= size) || (y >= size)) {
                if (wraparound) {
                    x = x % size;
                    y = y % size;
                } else {
                    bg_buffer[id][i] = 0;
                    continue;
                }
            }
            if (x < 0) {
                if (wraparound) {
                    x = (size + x) % size;
                } else {
                    bg_buffer[id][i] = 0;
                    continue;
                }
            }
            if (y < 0) {
                if (wraparound) {
                    y = (size + y) % size;
                } else {
                    bg_buffer[id][i] = 0;
                    continue;
                }
            }
            
            tile_internal_line = y % 8;
            tile_internal_x = x % 8;
            tile_row = (y - tile_internal_line) / 8;
            tile_column = (x - tile_internal_x) / 8;
            tile_number = vram[map_block_base + tile_row * blocks + tile_column];
            bg_buffer[id][i] = DecodeTilePixel8BPP(tile_block_base, tile_number, tile_internal_line, tile_internal_x, false);
        }
    }
    
    void GBAVideo::RenderSprites(int priority, int line, u32 tile_base)
    {
        // Process OBJ127 first, because OBJ0 has highest priority (OBJ0 overlays OBJ127, not vice versa)
        u32 offset = 127 * 8;
        bool one_dimensional = gba_io->dispcnt & (1 << 6) ? true : false;

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
                if (line >= y && line <= y + height - 1)
                {
                    int internal_line = line - y;
                    int displacement_y;
                    int row;
                    int tiles_per_row = width / 8;
                    int tile_number = attribute2 & 0x3FF;
                    int palette_number = attribute2 >> 12;
                    bool rotate_scale = attribute0 & (1 << 8) ? true : false;
                    bool horizontal_flip = !rotate_scale && (attribute1 & (1 << 12));
                    bool vertical_flip = !rotate_scale && (attribute1 & (1 << 13));
                    bool color_mode = attribute0 & (1 << 13) ? true : false;
                    
                    // It seems like the tile number is divided by two in 256 color mode
                    // Though we should check this because I cannot find information about this in GBATEK
                    if (color_mode)
                    {
                        tile_number /= 2;
                    }

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
                            current_tile_number = tile_number + row * tiles_per_row + j;
                        }
                        else
                        {
                            current_tile_number = tile_number + row * 32 + j;
                        }

                        // Render either in 256 colors or 16 colors mode
                        if (color_mode)
                        {
                            // 256 colors
                            tile_data = DecodeTileLine8BPP(tile_base, current_tile_number, displacement_y, true);
                        }
                        else
                        {
                            // 16 colors (use palette_nummer)
                            tile_data = DecodeTileLine4BPP(tile_base, 0x200 + palette_number * 0x20, current_tile_number, displacement_y);
                        }

                        // Copy data
                        if (horizontal_flip) // not very beautiful but faster
                        {
                            for (int k = 0; k < 8; k++)
                            {
                                int dst_index = x + (tiles_per_row - j - 1) * 8 + (7 - k);
                                u32 color = tile_data[k];
                                if ((color >> 24) != 0 && dst_index < 240)
                                {
                                    obj_buffer[priority][dst_index] = color;
                                }
                            }

                        }
                        else
                        {
                            for (int k = 0; k < 8; k++)
                            {
                                int dst_index = x + j * 8 + k;
                                u32 color = tile_data[k];
                                if ((color >> 24) != 0 && dst_index < 240)
                                {
                                    obj_buffer[priority][dst_index] = tile_data[k];
                                }
                            }
                        }

                        // We don't need that memory anymore
                        delete[] tile_data;
                    }
                }
            }

            // Update offset to the next entry
            offset -= 8;
        }
    }
    
    void GBAVideo::Render(int line)
    {
        int mode = gba_io->dispcnt & 7;
        int win_left[2];
        int win_right[2];
        int win_top[2];
        int win_bottom[2];
        bool win_enable[2];
        bool win_obj_enable;
        bool win_none;
        bool bg_winout[4];
        int bg_priority[4];
        bool bg_enable[4];
        bool obj_enable;
        bool first_bg = true;
        
        // Decode win display/enable values
        win_enable[0] = gba_io->dispcnt & (1 << 13);
        win_enable[1] = gba_io->dispcnt & (1 << 14);
        win_obj_enable = gba_io->dispcnt & (1 << 15);
        win_none = !win_enable[0] && !win_enable[1] && !win_obj_enable;
        
        if (win_enable[0]) {
            // Decode window0 coordinates
            win_left[0] = gba_io->win0h >> 8;
            win_right[0] = (gba_io->win0h & 0xFF) - 1;
            win_bottom[0] = (gba_io->win0v & 0xFF) - 1;
            win_top[0] = gba_io->win0v >> 8;

            // Fix window0 garbage values
            if (win_right[0] > 240 || win_left[0] > win_right[0]) win_right[0] = 240;
            if (win_bottom[0] > 160 || win_top[0] > win_bottom[0]) win_bottom[0] = 160;
        }
        
        if (win_enable[1]) {
            // Decode window1 coordinates
            win_left[1] = gba_io->win1h >> 8;
            win_right[1] = (gba_io->win1h & 0xFF) - 1;
            win_bottom[1] = (gba_io->win1v & 0xFF) - 1;
            win_top[1] = gba_io->win1v >> 8;
            
            // Fix window1 garbage values
            if (win_right[1] > 240 || win_left[1] > win_right[1]) win_right[1] = 240;
            if (win_bottom[1] > 160 || win_top[1] > win_bottom[1]) win_bottom[1] = 160;
        }
        
        // Decode WINOUT bg display
        if (win_enable[0] || win_enable[1] || win_obj_enable) {
            bg_winout[0] = gba_io->winout & 1;
            bg_winout[1] = gba_io->winout & 2;
            bg_winout[2] = gba_io->winout & 4;
            bg_winout[3] = gba_io->winout & 8;
        }
        
        // Decode background priorities
        // TODO: Decode only if background is enabled (extra boost)
        bg_priority[0] = *bgcnt[0] & 3;
        bg_priority[1] = *bgcnt[1] & 3;
        bg_priority[2] = *bgcnt[2] & 3;
        bg_priority[3] = *bgcnt[3] & 3;
        
        // Find out which layers are enabled
        bg_enable[0] = gba_io->dispcnt & (1 << 8) ? true : false;
        bg_enable[1] = gba_io->dispcnt & (1 << 9) ? true : false;
        bg_enable[2] = gba_io->dispcnt & (1 << 10) ? true : false;
        bg_enable[3] = gba_io->dispcnt & (1 << 11) ? true : false;
        obj_enable = gba_io->dispcnt & (1 << 12) ? true : false;
        
        // Reset obj buffers
        memset(obj_buffer[0], 0, 4*240);
        memset(obj_buffer[1], 0, 4*240);
        memset(obj_buffer[2], 0, 4*240);
        memset(obj_buffer[3], 0, 4*240);

        // Emulate the effect caused by "Forced Blank"
        if (gba_io->dispcnt & (1 << 7)) {
            for (int i = 0; i < 240; i++) {
                buffer[line * 240 + i] = 0xFFF8F8F8;
            }
            return;
        }

        // Call mode specific rendering logic
        switch (mode) {
        case 0:
        {
            // BG Mode 0 - 240x160 pixels, Text mode
            // TODO: Consider using no loop
            for (int i = 0; i < 4; i++) {
                if (bg_enable[i]) {
                    RenderBackgroundMode0(i, line);
                }
            }
            break;
        }
        case 1:
        {        
            // BG Mode 1 - 240x160 pixels, Text and RS mode mixed
            if (bg_enable[0]) {
                RenderBackgroundMode0(0, line);
            }
            if (bg_enable[1]) {
                RenderBackgroundMode0(1, line);
            }
            if (bg_enable[2]) {
                RenderBackgroundMode1(2, line);
            }
            break;
        }
        case 2:
        {
            // BG Mode 2 - 240x160 pixels, RS mode
            if (bg_enable[2]) {
                RenderBackgroundMode1(2, line);
            }
            if (bg_enable[3]) {
                RenderBackgroundMode1(3, line);
            }
            break;
        }
        case 3:
            // BG Mode 3 - 240x160 pixels, 32768 colors
            // Bitmap modes are rendered on BG2 which means we must check if it is enabled
            if (bg_enable[2])
            {
                u32 offset = line * 240 * 2;
                for (int x = 0; x < 240; x++)
                {
                    bg_buffer[2][line * 240 + x] = DecodeRGB5((vram[offset + 1] << 8) | vram[offset]);
                    offset += 2;
                }
            }
            break;
        case 4:
            // BG Mode 4 - 240x160 pixels, 256 colors (out of 32768 colors)
            // Bitmap modes are rendered on BG2 which means we must check if it is enabled
            if (bg_enable[2])
            {
                u32 page = gba_io->dispcnt & 0x10 ? 0xA000 : 0;
                for (int x = 0; x < 240; x++)
                {
                    u8 index = vram[page + line * 240 + x];
                    u16 rgb5 = pal[index * 2] | (pal[index * 2 + 1] << 8);
                    bg_buffer[2][line * 240 + x] = DecodeRGB5(rgb5);
                }
            }
            break;
        case 5:
            // BG Mode 5 - 160x128 pixels, 32768 colors
            // Bitmap modes are rendered on BG2 which means we must check if it is enabled
            if (bg_enable[2])
            {
                u32 offset = (gba_io->dispcnt & 0x10 ? 0xA000 : 0) + line * 160 * 2;
                for (int x = 0; x < 240; x++)
                {
                    if (x < 160 && line < 128)
                    {
                        bg_buffer[2][line * 240 + x] = DecodeRGB5((vram[offset + 1] << 8) | vram[offset]);
                        offset += 2;
                    }
                    else
                    {
                        // The unused space is filled with the first color from pal ram as far as I can see
                        u16 rgb5 = pal[0] | (pal[1] << 8);
                        bg_buffer[2][line * 240 + x] = DecodeRGB5(rgb5);
                    }
                }
            }
            break;
        }
        
        // Check if objects are enabled..
        if (obj_enable) {
            // .. and render all of them to their buffers if so
            RenderSprites(0, line, 0x10000);
            RenderSprites(1, line, 0x10000);
            RenderSprites(2, line, 0x10000);
            RenderSprites(3, line, 0x10000);
        }
    
        // Compose the picture *outside* the window
        for (int i = 3; i >= 0; i--) {
            for (int k = 3; k >= 0; k--) {
                // Is background visible and priority matches?
                if (bg_enable[k] && bg_priority[k] == i) {
                    // Draw line
                    if (win_none || gba_io->winout & (1 << k)) {
                        DrawLineToBuffer(bg_buffer[k], line, first_bg);
                    }
                    first_bg = false;
                }
            }
            if (obj_enable && (win_none || gba_io->winout & (1 << 4))) {
                DrawLineToBuffer(obj_buffer[i], line, false);
            }
        }
        
        // Compose window[0/1]
        for (int i = 1; i >= 0; i--) {
            if (win_enable[i] && line >= win_top[i] && line <= win_bottom[i]) {
                for (int j = 3; j >= 0; j--) {
                    for (int k = 3; k >= 0; k--) {
                        // Is background visible and priority matches?
                        if (bg_enable[k] && bg_priority[k] == j && 
                            (gba_io->winin & (1 << (k + ((i == 1) ? 8 : 0))))) {
                            u32 inside[240];

                            // Clear buffer and copy visible area
                            memset(inside, 0, sizeof(u32) * 240);
                            memcpy((u32*)(inside + win_left[i]), (u32*)(bg_buffer[k] + win_left[i]), sizeof(u32) * (win_right[i] - win_left[i] + 1));

                            // Draw line
                            DrawLineToBuffer(inside, line, first_bg);
                            first_bg = false;
                        }
                    }
                    if (obj_enable && 
                        (gba_io->winin & (1 << ((i == 1) ? 12 : 4)))) {
                        DrawLineToBuffer(obj_buffer[j], line, false);
                    }
                }
            }
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
                hblank_dma = true;
                state = GBAVideoState::HBlank;
                
                // Increment bg2x, bg2y, bg3x and bg3y by corresponding dmx/dmy after each scanline
                //bg2x_internal = EncodeGBAFloat32(DecodeGBAFloat32(bg2x_internal) + DecodeGBAFloat16(gba_io->bg2pb));
                //bg2y_internal = EncodeGBAFloat32(DecodeGBAFloat32(bg2y_internal) + DecodeGBAFloat16(gba_io->bg2pd));
                //bg3x_internal = EncodeGBAFloat32(DecodeGBAFloat32(bg3x_internal) + DecodeGBAFloat16(gba_io->bg3pb));
                //bg3y_internal = EncodeGBAFloat32(DecodeGBAFloat32(bg3y_internal) + DecodeGBAFloat16(gba_io->bg3pd));
                
                if (hblank_irq_enable)
                {
                    gba_io->if_ |= 2;
                }

                Render(gba_io->vcount);
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
                    bgx_int[2] = gba_io->bg2x;
                    bgy_int[2] = gba_io->bg2y;
                    bgx_int[3] = gba_io->bg3x;
                    bgy_int[3] = gba_io->bg3y;
                    vblank_dma = true;
                    state = GBAVideoState::VBlank;
                }
                else
                {
                    hblank_dma = false;
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
                    vblank_dma = false;
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