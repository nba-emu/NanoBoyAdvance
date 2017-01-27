///////////////////////////////////////////////////////////////////////////////////
//
//  NanoboyAdvance is a modern Game Boy Advance emulator written in C++
//  with performance, platform independency and reasonable accuracy in mind.
//  Copyright (C) 2017 Frederic Meyer
//
//  This file is part of nanoboyadvance.
//
//  nanoboyadvance is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 2 of the License, or
//  (at your option) any later version.
//
//  nanoboyadvance is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with nanoboyadvance. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////////////

#include "ppu.hpp"
#include "util/logger.hpp"

using namespace util;

namespace gba
{
    ppu::ppu()
    {
        reset();
    }

    void ppu::reset()
    {
        // reset DISPSTAT
        m_io.status.vblank_flag = false;
        m_io.status.hblank_flag = false;
        m_io.status.vcount_flag = false;
        m_io.status.vblank_interrupt = false;
        m_io.status.hblank_interrupt = false;
        m_io.status.vcount_interrupt = false;
        m_io.status.vcount_setting = 0;

        // reset vertical counter
        m_io.vcount = 0;
    }

    u32* ppu::get_framebuffer()
    {
        return m_framebuffer;
    }

    void ppu::set_memory(u8* pal, u8* oam, u8* vram)
    {
        m_pal = pal;
        m_oam = oam;
        m_vram = vram;
    }

    void ppu::set_interrupt(interrupt* interrupt)
    {
        m_interrupt = interrupt;
    }

    void ppu::hblank()
    {
        m_io.status.vblank_flag = false;
        m_io.status.hblank_flag = true;

        if (m_io.status.hblank_interrupt)
            m_interrupt->request(INTERRUPT_HBLANK);
    }

    void ppu::vblank()
    {
        m_io.status.vblank_flag = true;
        m_io.status.hblank_flag = false;

        if (m_io.vcount == 0) // eh...
        {
            if (m_io.status.vblank_interrupt)
                m_interrupt->request(INTERRUPT_VBLANK);
        }
        else if (m_io.vcount == 227)
        {

        }
        else
        {

        }
    }

    void ppu::scanline()
    {
        for (int x = 0; x < 240; x++)
        {
            int index = m_vram[m_io.vcount * 240 + x];
            u16 abgr = m_pal[index << 1] | (m_pal[(index << 1) + 1] << 8);
            u32 argb = 0xFF000000;

            argb |= ((abgr & 0x1F) << 3) << 16;
            argb |= (((abgr >> 5) & 0x1F) << 3) << 8;
            argb |= ((abgr >> 10) & 0x1F) << 3;

            m_framebuffer[m_io.vcount * 240 + x] = argb;
        }
    }

    void ppu::next_line()
    {
        bool vcount_flag = m_io.vcount == m_io.status.vcount_setting;
        m_io.vcount = (m_io.vcount + 1) % 228;
        m_io.status.vcount_flag = vcount_flag;

        if (vcount_flag && m_io.status.vcount_interrupt)
            m_interrupt->request(INTERRUPT_VCOUNT);
    }
}
