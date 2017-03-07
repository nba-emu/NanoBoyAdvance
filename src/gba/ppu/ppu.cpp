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
#include "util/logger.hpp"

using namespace Util;

namespace GameBoyAdvance {
    
    PPU::PPU() {
        reset();
        m_frameskip = 0;
    }

    void PPU::reset() {
        
        m_io.vcount = 0;

        // reset DISPCNT and DISPCNT
        m_io.control.reset();
        m_io.status.reset();

        // reset all backgrounds
        for (int i = 0; i < 4; i++) {
            
            m_io.bghofs[i] = 0;
            m_io.bgvofs[i] = 0;
            m_io.bgcnt[i].reset();
            
            if (i < 2) {
                m_io.bgx[i].reset();
                m_io.bgy[i].reset();
                m_io.bgpa[i] = 0;
                m_io.bgpb[i] = 0;
                m_io.bgpc[i] = 0;
                m_io.bgpd[i] = 0;
            }
        }
        
        // reset blending registers
        m_io.bldcnt.reset();
        m_io.bldalpha.reset();
        m_io.bldy.reset();

        m_frame_counter = 0;
    }

    u32* PPU::get_framebuffer() {
        return m_framebuffer;
    }

    void PPU::set_frameskip(int frames) {
        m_frameskip = frames;
    }

    void PPU::set_memory(u8* pal, u8* oam, u8* vram) {
        m_pal  = pal;
        m_oam  = oam;
        m_vram = vram;
    }

    void PPU::set_interrupt(Interrupt* interrupt) {
        m_interrupt = interrupt;
    }

    void PPU::hblank() {
        
        m_io.status.vblank_flag = false;
        m_io.status.hblank_flag = true;

        if (m_io.status.hblank_interrupt)
            m_interrupt->request(INTERRUPT_HBLANK);
    }

    void PPU::vblank() {
        
        m_io.status.vblank_flag = true;
        m_io.status.hblank_flag = false;

        m_io.bgx[0].internal = PPU::decode_float32(m_io.bgx[0].value);
        m_io.bgy[0].internal = PPU::decode_float32(m_io.bgy[0].value);
        m_io.bgx[1].internal = PPU::decode_float32(m_io.bgx[1].value);
        m_io.bgy[1].internal = PPU::decode_float32(m_io.bgy[1].value);

        if (m_frameskip != 0) {
            m_frame_counter = (m_frame_counter + 1) % m_frameskip;
        }

        if (m_io.status.vblank_interrupt) {
            m_interrupt->request(INTERRUPT_VBLANK);
        }
    }

    void PPU::scanline() {
        
        // todo: maybe find a better way
        m_io.status.vblank_flag = false;
        m_io.status.hblank_flag = false;

        // off by one scanline?
        m_io.bgx[0].internal += PPU::decode_float16(m_io.bgpb[0]);
        m_io.bgy[0].internal += PPU::decode_float16(m_io.bgpd[0]);
        m_io.bgx[1].internal += PPU::decode_float16(m_io.bgpb[1]);
        m_io.bgy[1].internal += PPU::decode_float16(m_io.bgpd[1]);

        if (m_frameskip == 0 || m_frame_counter == 0) {
            
            if (m_io.control.forced_blank) {
                u32* line = m_framebuffer + m_io.vcount * 240;
            
                for (int x = 0; x < 240; x++) {
                    line[x] = color_convert(0x7FFF);
                }
                return;
            }

            switch (m_io.control.mode) {
                case 0:
                    // BG Mode 0 - 240x160 pixels, Text mode
                    if (m_io.control.enable[0]) render_text(0);
                    if (m_io.control.enable[1]) render_text(1);
                    if (m_io.control.enable[2]) render_text(2);
                    if (m_io.control.enable[3]) render_text(3);
                    break;
                case 1:
                    // BG Mode 1 - 240x160 pixels, Text and RS mode mixed
                    if (m_io.control.enable[0]) render_text(0);
                    if (m_io.control.enable[1]) render_text(1);
                    if (m_io.control.enable[2]) render_affine(0);
                    break;
                case 2:
                    // BG Mode 2 - 240x160 pixels, RS mode
                    if (m_io.control.enable[2]) render_affine(0);
                    if (m_io.control.enable[2]) render_affine(1);
                    break;
                case 3:
                    // BG Mode 3 - 240x160 pixels, 32768 colors
                    if (m_io.control.enable[2]) {
                        render_bitmap_1();
                    }
                    break;
                case 4:
                    // BG Mode 4 - 240x160 pixels, 256 colors (out of 32768 colors)
                    if (m_io.control.enable[2]) {
                        render_bitmap_2();
                    }
                    break;
                case 5:
                    // BG Mode 5 - 160x128 pixels, 32768 colors
                    if (m_io.control.enable[2]) {
                        render_bitmap_3();
                    }
                    break;
                default:
                    Logger::log<LOG_ERROR>("unknown ppu mode: {0}", m_io.control.mode);
            }

            if (m_io.control.enable[4]) {
                // TODO(accuracy): tile base might not always be 0x10000?
                render_obj(0x10000);
            }

            compose_scanline();
        }
    }

    void PPU::next_line() {
        
        bool vcount_flag = m_io.vcount == m_io.status.vcount_setting;
        
        m_io.vcount = (m_io.vcount + 1) % 228;
        m_io.status.vcount_flag = vcount_flag;

        if (vcount_flag && m_io.status.vcount_interrupt) {
            m_interrupt->request(INTERRUPT_VCOUNT);
        }
    }
}
