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

#include "video.h"

/* TODO: 1) Improve RenderSprites method and allow for OBJWIN
 *          mask generation
 *       2) Implement OBJWIN
 *       3) Lookup if offset to RenderSprites is always constant
 *       4) Fix rotate-scale logic and apply to RenderSprites and Mode3-5
 */

namespace NanoboyAdvance
{
    GBAVideo::GBAVideo(GBAInterrupt* interrupt)
    {
        // Assign interrupt struct to video device
        this->interrupt = interrupt;

        // Zero init memory buffers
        memset(pal, 0, 0x400);
        memset(vram, 0, 0x18000);
        memset(obj, 0, 0x400);
        memset(buffer, 0, 240 * 160 * 4);
    }

    void GBAVideo::RenderBackgroundMode0(int id, int line)
    {
        // IO-register values
        u16 scx = bg[id].x;
        u16 scy = bg[id].y;
        
        // Rendering variables
        u32 tile_block_base = bg[id].tile_base;
        u32 map_block_base = bg[id].map_base;
        bool color_mode = bg[id].true_color;
        int width = ((bg[id].size & 1) + 1) << 8;
        int height = ((bg[id].size >> 1) + 1) << 8;
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
        // Rendering variables
        u32 tile_block_base = bg[id].tile_base;
        u32 map_block_base = bg[id].map_base;
        bool wraparound = bg[id].wraparound;
        int blocks = ((bg[id].size) + 1) << 4;
        int size = blocks * 8;
        
        for (int i = 0; i < 240; i++) {
            float dec_bgx = bg_x_int[id];
            float dec_bgy = bg_y_int[id];
            float dec_bgpa = DecodeGBAFloat16(bg[id].pa);
            float dec_bgpb = DecodeGBAFloat16(bg[id].pb);
            float dec_bgpc = DecodeGBAFloat16(bg[id].pc);
            float dec_bgpd = DecodeGBAFloat16(bg[id].pd);
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
                        if (oam_mapping)
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

    u32* GBAVideo::ApplySFX(int buffer_id)
    {
        return nullptr;        
    }
    
    void GBAVideo::Render(int line)
    {
        bool first_bg = true;
        bool win_none = !win_enable[0] && !win_enable[1] && !obj_win_enable;
        
        // Reset obj buffers
        memset(obj_buffer[0], 0, sizeof(u32)*240);
        memset(obj_buffer[1], 0, sizeof(u32)*240);
        memset(obj_buffer[2], 0, sizeof(u32)*240);
        memset(obj_buffer[3], 0, sizeof(u32)*240);

        // Emulate the effect caused by "Forced Blank"
        if (forced_blank) {
            for (int i = 0; i < 240; i++) {
                buffer[line * 240 + i] = 0xFFF8F8F8;
            }
            return;
        }

        // Call mode specific rendering logic
        switch (video_mode) {
        case 0:
        {
            // BG Mode 0 - 240x160 pixels, Text mode
            // TODO: Consider using no loop
            for (int i = 0; i < 4; i++) {
                if (bg[i].enable) {
                    RenderBackgroundMode0(i, line);
                }
            }
            break;
        }
        case 1:
        {        
            // BG Mode 1 - 240x160 pixels, Text and RS mode mixed
            if (bg[0].enable) {
                RenderBackgroundMode0(0, line);
            }
            if (bg[1].enable) {
                RenderBackgroundMode0(1, line);
            }
            if (bg[2].enable) {
                RenderBackgroundMode1(2, line);
            }
            break;
        }
        case 2:
        {
            // BG Mode 2 - 240x160 pixels, RS mode
            if (bg[2].enable) {
                RenderBackgroundMode1(2, line);
            }
            if (bg[3].enable) {
                RenderBackgroundMode1(3, line);
            }
            break;
        }
        case 3:
            // BG Mode 3 - 240x160 pixels, 32768 colors
            // Bitmap modes are rendered on BG2 which means we must check if it is enabled
            if (bg[2].enable)
            {
                u32 offset = line * 240 * 2;
                for (int x = 0; x < 240; x++)
                {
                    bg_buffer[2][x] = DecodeRGB5((vram[offset + 1] << 8) | vram[offset]);
                    offset += 2;
                }
            }
            break;
        case 4:
            // BG Mode 4 - 240x160 pixels, 256 colors (out of 32768 colors)
            // Bitmap modes are rendered on BG2 which means we must check if it is enabled
            if (bg[2].enable)
            {
                u32 page = frame_select ? 0xA000 : 0;
                for (int x = 0; x < 240; x++)
                {
                    u8 index = vram[page + line * 240 + x];
                    u16 rgb5 = pal[index * 2] | (pal[index * 2 + 1] << 8);
                    bg_buffer[2][x] = DecodeRGB5(rgb5);
                }
            }
            break;
        case 5:
            // BG Mode 5 - 160x128 pixels, 32768 colors
            // Bitmap modes are rendered on BG2 which means we must check if it is enabled
            if (bg[2].enable)
            {
                u32 offset = (frame_select ? 0xA000 : 0) + line * 160 * 2;
                for (int x = 0; x < 240; x++)
                {
                    if (x < 160 && line < 128)
                    {
                        bg_buffer[2][x] = DecodeRGB5((vram[offset + 1] << 8) | vram[offset]);
                        offset += 2;
                    }
                    else
                    {
                        // The unused space is filled with the first color from pal ram as far as I can see
                        u16 rgb5 = pal[0] | (pal[1] << 8);
                        bg_buffer[2][x] = DecodeRGB5(rgb5);
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
    
        // Compose screen
        if (win_none) {
            for (int i = 3; i >= 0; i--) {
                for (int j = 3; j >= 0; j--) {
                    if (bg[j].enable && bg[j].priority == i) {
                        DrawLineToBuffer(bg_buffer[j], line, first_bg);
                        first_bg = false;
                    }
                }
                if (obj_enable) {
                    DrawLineToBuffer(obj_buffer[i], line, false);
                }
            }
        } else {
            // Compose outer window area
            for (int i = 3; i >= 0; i--) {
                for (int j = 3; j >= 0; j--) {
                    if (bg[j].enable && bg[j].priority == i && bg_winout[j]) {
                        DrawLineToBuffer(bg_buffer[j], line, first_bg);
                        first_bg = false;
                    }
                }
                if (obj_enable && obj_winout) {
                    DrawLineToBuffer(obj_buffer[i], line, false);
                }
            }
            
            // Compose inner window[0/1] area
            for (int i = 1; i >= 0; i--) {
                if (win_enable[i] && (
                    (win_top[i] <= win_bottom[i] && line >= win_top[i] && line <= win_bottom[i]) ||
                    (win_top[i] > win_bottom[i] && !(line <= win_top[i] && line >= win_bottom[i]))
                )) {
                    u32 win_buffer[240];
                    
                    // Anything that is not covered by bg and obj will be black
                    for (int j = 0; j < 240; j++) {
                        win_buffer[j] = 0xFF000000;
                    }

                    // Draw backgrounds and sprites if any
                    for (int j = 3; j >= 0; j--) {
                        for (int k = 3; k >= 0; k--) {
                            if (bg[k].enable && bg[k].priority == j && bg_winin[i][k]) {
                                OverlayLineBuffers(win_buffer, bg_buffer[k]);
                            }
                        }
                        if (obj_enable && obj_winin[i]) {
                            OverlayLineBuffers(win_buffer, obj_buffer[j]);
                        }
                    }

                    // Make the window buffer transparent in the outer area
                    if (win_left[i] <= win_right[i] + 1) {
                        for (int j = 0; j <= win_left[i]; j++) {
                            if (j < 240) {
                                win_buffer[j] = 0;
                            }
                        }
                        for (int j = win_right[i]; j < 240; j++) {
                            if (j >= 0) {
                                win_buffer[j] = 0;
                            }
                        }
                    } else {
                        for (int j = win_right[i]; j <= win_left[i]; j++) {
                            if (j >= 0 && j < 240) {
                                win_buffer[j] = 0;
                            }
                        }
                    }
                    
                    // Draw the window buffer
                    DrawLineToBuffer(win_buffer, line, false);
                }
            }
        }
    }

    void NanoboyAdvance::GBAVideo::Step()
    {
        // Update tickcount
        ticks++;

        // Reset flags
        render_scanline = false;

        // Handle V-Count Setting (LYC)
        vcount_flag = vcount == vcount_setting;

        switch (state)
        {
        case GBAVideoState::Scanline:
        {
            if (ticks >= 960)
            {
                hblank_dma = true;
                state = GBAVideoState::HBlank;
                
                // Increment bg2x, bg2y, bg3x and bg3y by corresponding dmx/dmy after each scanline
                //bg2x_internal = EncodeGBAFloat32(DecodeGBAFloat32(bg2x_internal) + DecodeGBAFloat16(gba_io->bg2pb));
                //bg2y_internal = EncodeGBAFloat32(DecodeGBAFloat32(bg2y_internal) + DecodeGBAFloat16(gba_io->bg2pd));
                //bg3x_internal = EncodeGBAFloat32(DecodeGBAFloat32(bg3x_internal) + DecodeGBAFloat16(gba_io->bg3pb));
                //bg3y_internal = EncodeGBAFloat32(DecodeGBAFloat32(bg3y_internal) + DecodeGBAFloat16(gba_io->bg3pd));
                
                if (hblank_irq)
                {
                    interrupt->if_ |= 2;
                }

                Render(vcount);
                render_scanline = true;
                
                ticks = 0;
            }
            break;
        }
        case GBAVideoState::HBlank:
            if (ticks >= 272)
            {
                vcount++;
                if (vcount_flag && vcount_irq)
                {
                    interrupt->if_ |= 4;
                }
                if (vcount == 160)
                {
                    bg_x_int[2] = DecodeGBAFloat32(bg[2].x_ref);
                    bg_y_int[2] = DecodeGBAFloat32(bg[2].y_ref);
                    bg_x_int[3] = DecodeGBAFloat32(bg[3].x_ref);
                    bg_y_int[3] = DecodeGBAFloat32(bg[3].y_ref);
                    hblank_dma = false;
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
            if (ticks >= 1232)
            {
                vcount++;
                if (vcount_flag && vcount_irq)
                {
                    interrupt->if_ |= 4;
                }
                if (vblank_irq && vcount == 161)
                {
                    interrupt->if_ |= 1;
                }
                if (vcount >= 227) // check wether this must be 227 or 228
                {
                    vblank_dma = false;
                    state = GBAVideoState::Scanline;
                    vcount = 0;
                }
                ticks = 0;
            }
            break;
        }
        }
    }
}
