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

#include "../ppu.hpp"
#include "util/logger.hpp"

namespace GameBoyAdvance {
    
    static constexpr int g_obj_size[4][4][2] = {
        /* SQUARE */
        {
            { 8 , 8  },
            { 16, 16 },
            { 32, 32 },
            { 64, 64 }
        },
        /* HORIZONTAL */
        {
            { 16, 8  },
            { 32, 8  },
            { 32, 16 },
            { 64, 32 }
        },
        /* VERTICAL */
        {
            { 8 , 16 },
            { 8 , 32 },
            { 16, 32 },
            { 32, 64 }
        },
        /* PROHIBITED */
        {
            { 0, 0 },
            { 0, 0 },
            { 0, 0 },
            { 0, 0 }
        }
    };

    void PPU::render_obj(u32 tile_base) {
        
        // affine 2x2 matrix
        u16 pa, pb, pc, pd;
        
        u32 offset = 127 << 3;

        // eh...
        for (int i = 0; i < 240; i++) {
            m_buffer[4][i] = COLOR_TRANSPARENT;
            m_buffer[5][i] = COLOR_TRANSPARENT;
            m_buffer[6][i] = COLOR_TRANSPARENT;
            m_buffer[7][i] = COLOR_TRANSPARENT;
            m_obj_semi[i] = false;
        }
        
        // clear OBJWIN mask if required
        if (m_io.control.win_enable[2]) {
            memset(m_win_mask[2], 0, 240);
        }

        // TODO(performance):
        // we have to read OAM data in descending order but that
        // might affect dcache performance? not sure about impact.
        for (int i = 0; i < 128; i++) {
            
            // TODO(performance): decode these on OAM writes already?
            u16 attribute0 = (m_oam[offset + 1] << 8) | m_oam[offset + 0];
            u16 attribute1 = (m_oam[offset + 3] << 8) | m_oam[offset + 2];
            u16 attribute2 = (m_oam[offset + 5] << 8) | m_oam[offset + 4];

            int width, height;
            s32 x = attribute1 & 0x1FF;
            int y = attribute0 & 0x0FF;
            int shape = attribute0 >> 14;
            int size  = attribute1 >> 14;
            int prio  = (attribute2 >> 10) & 3;
            int mode  = (attribute0 >> 10) & 3;
            
            if (mode == OBJ_PROHIBITED) {
                offset -= 8;
                continue;
            }

            // can this be done more efficiently?
            if (x >= 240) x -= 512;            
            if (y >= 160) y -= 256;
            
            bool affine    = attribute0 & 256;
            bool attr0bit9 = attribute0 & (1<<9);
            
            // check if OBJ is disabled
            if (!affine && attr0bit9) {
                offset -= 8;
                continue;
            }

            // get width and height of OBJ
            width  = g_obj_size[shape][size][0];
            height = g_obj_size[shape][size][1];
            
            int rect_width  = width;
            int rect_height = height;
            
            // move x/y to the *center* of the OBJ
            x += width  >> 1;
            y += height >> 1;
            
            // read rot/scale parameters
            if (affine) {
                int group = ((attribute1 >> 9) & 0x1F) << 5;

                pa = (m_oam[group + 0x7 ] << 8) | m_oam[group + 0x6 ];
                pb = (m_oam[group + 0xF ] << 8) | m_oam[group + 0xE ];
                pc = (m_oam[group + 0x17] << 8) | m_oam[group + 0x16];
                pd = (m_oam[group + 0x1F] << 8) | m_oam[group + 0x1E];
                
                // double-size bit
                if (attr0bit9) {
                    x += width  >> 1;
                    y += height >> 1;
                    
                    rect_width  <<= 1;
                    rect_height <<= 1;
                }
                
            } else {
                // initialize P with identity matrix:
                // [ 1  0 ]
                // [ 0  1 ]
                pa = 0x0100;
                pb = 0x0000;
                pc = 0x0000;
                pd = 0x0100;
            }
            
            // half the width and height of OBJ screen area
            int half_width  = rect_width  >> 1;
            int half_height = rect_height >> 1;

            int line  = m_io.vcount;
            int min_y = y - half_height;
            int max_y = y + half_height;
            
            if (line >= min_y && line < max_y) {
                
                int rect_y = line - y;
                 
                int number  = attribute2 & 0x3FF;
                int palette = (attribute2 >> 12) + 16;
                bool h_flip = !affine && (attribute1 & (1 << 12));
                bool v_flip = !affine && (attribute1 & (1 << 13));
                bool is_256 = attribute0 & (1 << 13); 
                
                if (is_256) number >>= 1;
                
                for (int rect_x = -half_width; rect_x < half_width; rect_x++) {

                    // get pixel eccetera...
                    int screen_x = x + rect_x;
                    
                    // meh...
                    if (screen_x >= 0 && screen_x < 240) {
                        
                        // texture coordinates
                        int tex_x = PPU::decode_float16(pa) * rect_x + PPU::decode_float16(pb) * rect_y + (width  >> 1);
                        int tex_y = PPU::decode_float16(pc) * rect_x + PPU::decode_float16(pd) * rect_y + (height >> 1);
                        
                        // are the coordinates inside the valid boundaries?
                        if (tex_x >= width || tex_y >= height || 
                            tex_x < 0      || tex_y < 0     ) { continue; }
                        
                        if (h_flip) {
                            tex_x = width  - tex_x;
                        }
                        
                        if (v_flip) {
                            tex_y = height - tex_y;
                        }
                        
                        int tile_x  = tex_x  & 7;
                        int tile_y  = tex_y  & 7;
                        int block_x = tex_x >> 3;
                        int block_y = tex_y >> 3;
                        
                        int tile_num = number;
                        
                        if (m_io.control.one_dimensional) {
                            tile_num += block_y * (width >> 3);
                        } else {
                            tile_num += block_y << (is_256? 4 : 5);
                        }
                        
                        tile_num += block_x;
                        
                        u16 pixel;
                        
                        if (is_256) {
                            pixel = get_tile_pixel_8bpp(tile_base, 16     , tile_num, tile_x, tile_y);
                        } else {
                            pixel = get_tile_pixel_4bpp(tile_base, palette, tile_num, tile_x, tile_y);
                        }

                        if (pixel != COLOR_TRANSPARENT) {
                            if (mode == OBJ_WINDOW) {
                                m_win_mask[2][screen_x] = 1;
                            } else {
                                if (mode == OBJ_SEMI) {
                                    m_obj_semi[screen_x] = true;
                                }
                                m_buffer[4 + prio][screen_x] = pixel;
                            }
                        }
                    }
                }
            }
            
            offset -= 8;
        }
    }
}
