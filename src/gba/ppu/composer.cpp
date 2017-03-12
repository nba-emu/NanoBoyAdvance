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

#include "ppu.hpp"

namespace GameBoyAdvance {
    
    void PPU::apply_sfx(u16* target1, u16 target2, SpecialEffect sfx) {
        
        int r1 = (*target1 >> 0 ) & 0x1F;
        int g1 = (*target1 >> 5 ) & 0x1F;
        int b1 = (*target1 >> 10) & 0x1F;
        
        switch (sfx) {
            case SFX_BLEND: {
                int eva = m_io.bldalpha.eva;
                int evb = m_io.bldalpha.evb;
            
                int r2  = (target2 >> 0 ) & 0x1F;
                int g2  = (target2 >> 5 ) & 0x1F;
                int b2  = (target2 >> 10) & 0x1F;
                float a = eva >= 16 ? 1.0 : (eva / 16.0);
                float b = evb >= 16 ? 1.0 : (evb / 16.0);
            
                // linear combination of both pixels
                r1 = a * r1 + b * r2;
                g1 = a * g1 + b * g2;
                b1 = a * b1 + b * b2;
            
                break;
            }
            case SFX_INCREASE: {
                int evy = m_io.bldy.evy;
            
                float brightness = evy >= 16 ? 1.0 : (evy / 16.0); 
            
                r1 = r1 + (31 - r1) * brightness;
                g1 = g1 + (31 - g1) * brightness;
                b1 = b1 + (31 - b1) * brightness;
            
                break;
            }
            case SFX_DECREASE: {
                int evy = m_io.bldy.evy;
            
                float brightness = evy >= 16 ? 1.0 : (evy / 16.0); 
            
                r1 = r1 - r1 * brightness;
                g1 = g1 - g1 * brightness;
                b1 = b1 - b1 * brightness;
            
                break;
            }
        }
        
        if (r1 > 31) r1 = 31;
        if (g1 > 31) g1 = 31;
        if (b1 > 31) b1 = 31;
        
        *target1 = (r1 << 0 ) |
                   (g1 << 5 ) |
                   (b1 << 10);
    }
    
    void PPU::compose_scanline() {
        
        auto win0in   = m_io.winin.enable[0];
        auto win1in   = m_io.winin.enable[1];
        auto objwinin = m_io.winout.enable[1];
        
        u16 backdrop_color = (m_pal[1] << 8) | m_pal[0];
        u32* line_buffer   = m_framebuffer  + m_io.vcount * 240;
        
        const bool inside[6][3] = {
            { win0in[LAYER_BG0], win1in[LAYER_BG0], objwinin[LAYER_BG0] },
            { win0in[LAYER_BG1], win1in[LAYER_BG1], objwinin[LAYER_BG1] },
            { win0in[LAYER_BG2], win1in[LAYER_BG2], objwinin[LAYER_BG2] },
            { win0in[LAYER_BG3], win1in[LAYER_BG3], objwinin[LAYER_BG3] },
            { win0in[LAYER_OBJ], win1in[LAYER_OBJ], objwinin[LAYER_OBJ] },
            { win0in[LAYER_SFX], win1in[LAYER_SFX], objwinin[LAYER_SFX] }
        };
       
        auto bgcnt   = m_io.bgcnt;
        auto outside = m_io.winout.enable[0];
        auto enable  = m_io.control.enable;

        for (int x = 0; x < 240; x++) {
            int layer[2] = { LAYER_BD, LAYER_BD };
            u16 pixel[2] = { backdrop_color, 0 };
            
            auto& obj = m_obj_layer[x];
            
            for (int j = 3; j >= 0; j--) {
                
                for (int k = 3; k >= 0; k--) {
                    bool visible = is_visible(x, inside[k], outside[k]);

                    if (enable[k] && bgcnt[k].priority == j && visible) {
                        
                        u16 new_pixel = m_buffer[k][x];
                        
                        if (new_pixel != COLOR_TRANSPARENT) {
                            layer[1] = layer[0];
                            layer[0] = k;
                            pixel[1] = pixel[0];
                            pixel[0] = new_pixel;
                        }
                    }
                }
                
                if (enable[LAYER_OBJ] && obj.prio == j && 
                    is_visible(x, inside[LAYER_OBJ], outside[LAYER_OBJ])) {

                    u16 new_pixel = obj.pixel;
                        
                    if (new_pixel != COLOR_TRANSPARENT) {
                        layer[1] = layer[0];
                        layer[0] = LAYER_OBJ;
                        pixel[1] = pixel[0];
                        pixel[0] = new_pixel;
                    }
                }
            }
            
            if (is_visible(x, inside[LAYER_SFX], outside[LAYER_SFX])) {
                
                SpecialEffect sfx = m_io.bldcnt.sfx;
                bool is_alpha_obj = layer[0] == LAYER_OBJ && obj.alpha;
                bool sfx_above    = m_io.bldcnt.targets[0][layer[0]] || is_alpha_obj;
                bool sfx_below    = m_io.bldcnt.targets[1][layer[1]];
                
                if (is_alpha_obj && sfx_below) {
                    sfx = SFX_BLEND;
                }
                
                if (sfx != SFX_NONE && sfx_above && (sfx_below || sfx != SFX_BLEND)) {
                    apply_sfx(&pixel[0], pixel[1], sfx);
                }
            }

            line_buffer[x] = color_convert(pixel[0]);
        }
    }
}
